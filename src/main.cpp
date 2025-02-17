#include "../include/config.h"
#include "../include/database.h"
#include "../include/data_parser.h"
#include "../include/server.h"

#include <iostream>

class Application
{
private:
    Configuration configuration;

    Socket *socket = nullptr;

    XML::Parse xmlParser;

    Sqlite::DatabaseManager *databaseManager = nullptr;

    std::thread serverThread;

    /* Thread to handle server stopping on user input.*/
    std::thread stopServerThread;

public:
    /* Runs a application , managing configuraion , socket creation , and input handling.*/
    void run(int argc, char *argv[])
    {
        /* Configuration program.*/
        configuration.config(argc, argv);

        /* Initialize and start the socket server.*/
        server();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (socket->isRunning()) {
            std::cout << "Server started. Press Enter to stop...\n\n";

            /*Start the thread that waits for user input to stop the server.*/
            stopServerThread = std::thread([this]() {
                /*Wait for user input*/
                std::cin.get();
                socket->stop();
            });

            /*Process incoming client data */
            processAndStoreClientData();

        } else

            std::cout << "Server failed to start. Check logs for "
                         "errors.\n";
    }
    /* Destructor cleans up resources and stops the server if running */
    ~Application()
    {
        if (socket->isRunning())
            socket->stop();

        if (serverThread.joinable())
            serverThread.join();

        if (stopServerThread.joinable())
            stopServerThread.join();

        if (socket)
            delete socket;
        if (databaseManager)
            delete databaseManager;
    }

private:
    /* Initializes the socket and starts the server in a separate thread */
    void server()
    {
        socket = new Socket(configuration.getServerConfig());
        serverThread = std::thread(&Socket::createSocket, socket);
    }

    /* Processes incoming client data and stores it in the database */
    void processAndStoreClientData()
    {
        databaseManager = new DatabaseManager(configuration.getDatabaseConfig());
        while (true)

        {
            std::unique_lock<std::mutex> lock(socket->getMutex());

            /*Wait for data from clients or server stop signal.*/
            socket->getCV().wait(lock, [this] {
                return ! socket->getWaitingClients().empty() || ! socket->isRunning();
            });

            // Break if server is stopped and no more waiting clients
            if (! socket->isRunning() && socket->getWaitingClients().empty())
                break;

            // Get client from waiting queue and pop it
            Client *client = socket->getWaitingClients().front();
            socket->getWaitingClients().pop();

            /*Start a thread to process the XML data from the client.*/
            std::thread parseThread(&XML::Parse::parseAndStoreXmlData, &xmlParser, client,
                                    databaseManager);

            parseThread.detach();
            /*Store the parsing thread in clien*/
            client->setParseThread(std::move(parseThread));
        }
    }
};

int main(int argc, char *argv[])
{
    Application app;
    app.run(argc, argv);

    return 0;
}

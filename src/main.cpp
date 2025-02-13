#include "../include/config.h"
#include "../include/database.h"
#include "../include/parseXml.h"
#include "../include/server.h"

#include <iostream>
class Application
{
private:
    /* instance of Configuration class */
    Configuration configuration;

    /* Object of socket*/
    Socket *socket = nullptr;

    /* Object of xmlParser */
    XmlParser xmlParser;

    /* Object of DatabaseManager */
    DatabaseManager *databaseManager = nullptr;

    /* Thread of server */
    std::thread serverThread;

    /* Thread of input*/
    std::thread stopServerThread;

public:
    /* Run program */
    void run(int argc, char *argv[])
    {
        /* Configuration program */
        configuration.config(argc, argv);

        /* Create socket */
        server();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (socket->isRunning()) {
            std::cout << "Server started. Press Enter to stop...\n\n";

            stopServerThread = std::thread([this]() {
                std::cin.get();
                socket->stop();
            });

            processAndStoreClientData();

        } else

            std::cout << "Server failed to start. Check logs for "
                         "errors.\n";
    }

    /* Destructor */
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
    /* Create socket */
    void server()
    {
        socket = new Socket(configuration.getServerConfig());
        serverThread = std::thread(&Socket::createSocket, socket);
    }

    /* Process client's data  */
    void processAndStoreClientData()
    {
        databaseManager = new DatabaseManager(configuration.getDatabaseConfig());
        while (true)

        {
            std::unique_lock<std::mutex> lock(socket->getMutex());

            socket->getCV().wait(lock, [this] {
                return ! socket->getWaitingClients().empty() || ! socket->isRunning();
            });

            if (! socket->isRunning() && socket->getWaitingClients().empty())
                break;

            Client *client = socket->getWaitingClients().front();
            socket->getWaitingClients().pop();

            std::thread parseThread(&XmlParser::parseAndStoreXmlData, &xmlParser, client,
                                    databaseManager);

            parseThread.detach();
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

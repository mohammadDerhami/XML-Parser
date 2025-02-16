#include "server.h"

/**
 * Implementation the "Socket" class
 */

/*
 * Creates a TCP socket , binds it to the specified address ,and starts listening for incominng
 * clients connections.
 */
void Socket::createSocket()
{
    try {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0) {
            throw SocketException("Error creating socket!!!\n");
        }

        sockaddr_in address = setup();
        int addressLen = sizeof(address);

        bindSocket(address, addressLen);
        isBound = true;

        listenForClients();
        isListening = true;

        acceptClient(address, addressLen);

    } catch (const SocketException &se) {
        std::cerr << "Error : " << se.what();
        if (isRunning())
            stop();
    }
}

/*
 * Accepts incoming client connections and creates a new Client	object
 * for each connected client. each client is handled in a new thread.
 */
void Socket::acceptClient(sockaddr_in address, int addressLen)
{
    while (true) {
        int newClientSocket = accept(sockfd, (struct sockaddr *) &address,
                                     (socklen_t *) &addressLen);
        if (newClientSocket < 0) {
            throw SocketException("Exception in accept!!!\n");
        }

        clientsNum++;

        Client *client = new Client(newClientSocket, clientsNum);

        std::thread clientThread(&Socket::handleClient, this, client);

        client->setThread(std::move(clientThread));

        std::lock_guard<std::mutex> lock(socketMtx);
        clients.push_back(client);
    }
}

/*
 * Handles communication with the connected client ,including reading data from the client ,
 * processing it and sending back results.
 * */
void Socket::handleClient(Client *client)
{
    int clientSocket = client->getClientSocket();
    printClientJoin(client);

    while (isRunning()) {
        std::string lengthMsg = "\nEnter the data length as 15 digits : \n";
        write(clientSocket, lengthMsg.c_str(), lengthMsg.length());

        int size = readDataSize(client);

        if (size <= 0)
            continue;

        std::string dataMsg = "\nEnter the data of size " + std::to_string(size) + " : \n";
        write(clientSocket, dataMsg.c_str(), dataMsg.length());

        /*
         * Buffer to store client's data
         * 1024 bytes for additional input
         */
        char buffer[size + 1024];
        if (int bytesRead = readData(client, buffer, size) <= 0) {
            continue;
        }

        client->setXmlData(buffer);

        pushToQueue(client);

        cv.notify_one();

        printClientData(client);

        {
            std::unique_lock<std::mutex> lock(client->getMutex());
            client->getCV().wait(lock, [client] { return client->getResultReady(); });
        }

        write(clientSocket, client->getResult().c_str(), client->getResult().length());

        client->reset();

        std::string continueMsg = "\nPress 'y' if you want to continue .\n";
        write(clientSocket, continueMsg.c_str(), continueMsg.length());

        char userChoice = readClientChoice(client);

        if (userChoice != 'y')
            break;
    }
    closeClient(client);
}

/*
 * Reads a single characters choice from the client to determine
 * whether to continue or stop processing.
 *
 * Note: The buffer size is set to 128 bytes,
 * although we only read one character from the buffer.
 * this is done to handle additional input from the client
 * that may come in after the character ,such as newline characters
 * or other inputs.
 */
char Socket::readClientChoice(Client *client)
{
    char buffer[128];
    int bytesRead = read(client->getClientSocket(), buffer, sizeof(buffer));
    if (bytesRead < 1) {
        std::string message = "Server stoped. \n";
        write(client->getClientSocket(), message.c_str(), message.length());

        return '\0';
    }

    return buffer[0];
}

/*
 * Reads the size of the data that the client intends to sent.
 * the size must be a 15 digits string.
 *
 * Note: The buffer size is set to 1024 bytes,
 * although we only read 15 characters from the buffer.
 * this is done to handle additional input from the client
 * that may come in after the character ,such as newline characters
 * or other inputs.
 */
int Socket::readDataSize(Client *client)
{
    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));

    int bytesRead = read(client->getClientSocket(), buffer, sizeof(buffer));
    if (bytesRead < 15) {
        std::string message = "Your input is less than 15 digits.\n";
        write(client->getClientSocket(), message.c_str(), message.length());
        return -1;
    }

    buffer[15] = '\0';
    try {
        int size = std::stoi(buffer);
        return size;

    } catch (const std::invalid_argument &e) {
        std::string invalidError = "Invalid argument cannot convert to integer.\n";
        write(client->getClientSocket(), invalidError.c_str(), invalidError.length());
        return -1;

    } catch (const std::out_of_range &e) {
        std::string outOfRangeError = "Out of range error: value is too large.\n";
        write(client->getClientSocket(), outOfRangeError.c_str(), outOfRangeError.length());
        return -1;
    }
}
/* Reads the data sent by the client with the specified size */
int Socket::readData(Client *client, char *buffer, int size)
{
    int totalRead = 0;

    while (totalRead < size) {
        int bytesRead = read(client->getClientSocket(), buffer + totalRead, size - totalRead);

        if (bytesRead < 0) {
            return -1;
        }
        totalRead += bytesRead;
    }

    /*Handle extra input*/
    int extraBytes = 0;
    ioctl(client->getClientSocket(), FIONREAD, &extraBytes);

    if (extraBytes > 0) {
        char trash[1024];

        while (extraBytes > 0) {
            int bytesToRead = std::min(extraBytes, (int) sizeof(trash));

            int bytesRead = read(client->getClientSocket(), trash, bytesToRead);
            if (bytesRead <= 0)
                break;

            extraBytes -= bytesRead;
        }
    }

    return totalRead;
}

/*Pushes the Client object to the waitingClients queue for processing.*/
void Socket::pushToQueue(Client *client)
{
    std::lock_guard<std::mutex> lock(socketMtx);
    waitingClients.push(client);
}

/* Stops the server by closing the socket and cleaning up resources. */
void Socket::stop()
{
    isBound = false;
    isListening = false;

    shutdown(sockfd, SHUT_RD);
    close(sockfd);
    sockfd = -1;

    cv.notify_one();

    std::cout << "Server stoped.\n";
}

/* Checks if the socket is currently running(bound and listening)*/
bool Socket::isRunning()
{
    return isBound && isListening && sockfd > 0;
}

/* Binds the socket to the specified address */
void Socket::bindSocket(sockaddr_in address, int addressLen)
{
    if (bind(sockfd, (struct sockaddr *) &address, addressLen) < 0) {
        throw SocketException("Bind failed!!!\n");
    }
}

/* Sets up the server socket to listen for incoming client connections.*/
void Socket::listenForClients()
{
    if (listen(sockfd, serverConfig.getMaxConnection()) < 0) {
        throw SocketException("Listen failed!!!\n");
    }
}

/* Configures the socket with specified IP address and port. */
sockaddr_in Socket::setup()
{
    sockaddr_in address;

    int addrLen = sizeof(address);
    memset(&address, 0, addrLen);

    address.sin_family = AF_INET;

    inet_pton(AF_INET, ip.c_str(), (void *) &address.sin_addr);

    address.sin_port = htons(port);

    return address;
}
/* Prints the XML data received from the client to the colsole. */
void Socket::printClientData(Client *client)
{
    coutMtx.lock();
    std::cout << "received from " << client->getId() << "\n" << client->getXmlData() << std::endl;
    coutMtx.unlock();
}

/*Prints a message indicating that a client has closed their connection. */
void Socket::printClientClose(Client *client)
{
    coutMtx.lock();
    std::cout << "client with id " << client->getId() << " closed.\n";
    coutMtx.unlock();
}
/*Prints a message indicating that a client has joined the server.*/
void Socket::printClientJoin(Client *client)
{
    coutMtx.lock();
    std::cout << "client with id  " << client->getId() << " joined.\n";
    coutMtx.unlock();
}
/*Closes the socket connection for the specified client and prints a close message.*/
void Socket::closeClient(Client *client)
{
    close(client->getClientSocket());
    client->setClientSocket(-1);
    printClientClose(client);
}

/**
 *
 * Implementation for "Client" class
 *
 */

/*Resets the client status for reuse by clearing results and data flags.*/
void Client::reset()
{
    std::lock_guard<std::mutex> lock(clientMtx);

    resultReady = false;
    result.clear();

    dataReady = false;
    xmlData.clear();
}


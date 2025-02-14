/**
 * \file server.h
 *
 * socket structure.
 *
 * \author MohammadDerhami
 *
 */

#ifndef SERVER_H
#define SERVER_H

#include "config.h"

#include <arpa/inet.h>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>
/*
 * @class client
 * @brief class for client
 */
class Client
{
private:
    int clientSocket = -1;

    /*Input data*/
    std::string xmlData;

    /*Id of client */
    int id;

    /*Thread of client*/
    std::thread clientThread;

    /*Thread of parse and store data */
    std::thread parseThread;

    /*Result*/
    std::string result;

    /*Mutex*/
    std::mutex clientMtx;

    /*Condition varible*/
    std::condition_variable cv;

    /*Indicates whether the result is ready for use*/
    bool resultReady = false;

    /*Indicates whether the data is ready for processing*/
    bool dataReady = false;

public:
    /*Constructor*/
    Client(int socket, int id) : clientSocket(socket), id(id)
    {
    }

    /*Destructor*/
    ~Client()
    {
        if (clientThread.joinable())
            clientThread.join();
        if (parseThread.joinable())
            parseThread.join();
    }

    /*Setter and Getter for resultReady*/
    bool getResultReady() const
    {
        return resultReady;
    }
    void setResultReady(bool resultReady)
    {
        this->resultReady = resultReady;
    }

    /*Setter and Getter for dataReady*/
    bool getDataReady() const
    {
        return dataReady;
    }
    void setDataReady(bool dataReady)
    {
        this->dataReady = dataReady;
    }

    /*Getter for mutex*/
    std::mutex &getMutex()
    {
        return clientMtx;
    }

    /*Getter for cv*/
    std::condition_variable &getCV()
    {
        return cv;
    }

    /*Setter and Getter for thread*/
    void setThread(std::thread &&thread)
    {
        clientThread = std::move(thread);
    }
    std::thread &getThread()
    {
        return clientThread;
    }

    /*Setter and Getter for parse thread */
    void setParseThread(std::thread &&thread)
    {
        parseThread = std::move(thread);
    }

    std::thread &getParseThread()
    {
        return parseThread;
    }

    /*Setter and Getter for id*/
    void setId(int id)
    {
        this->id = id;
    }
    int getId() const
    {
        return id;
    }

    /*Setter and Getter for clientSocket*/
    void setClientSocket(int clientSocket)
    {
        this->clientSocket = clientSocket;
    }
    int getClientSocket() const
    {
        return clientSocket;
    }

    /*Setter and Getter for xmlData */
    std::string getXmlData()
    {
        std::lock_guard<std::mutex> lock(clientMtx);
        return xmlData;
    }
    void setXmlData(const std::string &xmlData)
    {
        std::lock_guard<std::mutex> lock(clientMtx);
        this->xmlData = xmlData;
        dataReady = true;
    }

    /*Getter for rusult*/
    std::string getResult()
    {
        std::lock_guard<std::mutex> lock(clientMtx);
        return result;
    }
    void setResult(const std::string &result)
    {
        std::lock_guard<std::mutex> lock(clientMtx);
        this->result = result;
        resultReady = true;
    }

    /* Resets the client state for reuse*/
    void reset();
};

/**
 * @ class socket
 * @ brief this class is responsible for creating  a socket
 */
class Socket
{
private:
    /* Server Configuration */
    ServerConfiguration serverConfig;

    /* IP */
    std::string ip;

    /* Port */
    int port;

    /*Socket discriptor*/
    int sockfd = -1;

    /*Indicates whether the socket is bound to a port.*/
    bool isBound = false;

    /*Indicates whether the socket is in a listening stat*/
    bool isListening = false;

    /*Mutex*/
    std::mutex socketMtx;
    std::mutex coutMtx;

    /*Condition variable*/
    std::condition_variable cv;

    /*Number of clients*/
    int clientsNum = 0;

    /*Vector to store clients*/
    std::vector<Client *> clients;

    /*Queue to store clients waiting for a response*/
    std::queue<Client *> waitingClients;

    /**
     *
     * Methods :
     *
     */

    /*
     * @brief converts the provided  config's IP address  and port  to a
     * native sockaddr_in structure.
     * @return a sockaddr_in structure ready for use in socket functions.
     */
    sockaddr_in setup();

    /*
     * @brief binding socket
     *
     * @param sockaddr , length of address
     *
     * @warning this method throws a SocketException if :
     * The socket binding failes.
     *
     * */
    void bindSocket(sockaddr_in address, int addressLen);

    /*
     * @brief listen on socket
     *
     *
     * @warning this method throws SocketException if:
     * The listening on the socket failes.
     */
    void listenForClients();

    /*
     * @brief accept client
     *
     * @param sockaddr_in , length of address
     *
     * @warning this method throws SocketException if:
     * Accepting incoming client connections fails.
     *
     */
    void acceptClient(sockaddr_in address, int addressLen);

    /*
     *@brief handle client -> read data length and read data.
     *@param instance of client
     */
    void handleClient(Client *client);

    /*
     * @brief print message (client joined)
     * @param instance of client
     */
    void printClientJoin(Client *client);

    /*
     * @brief reads data from the socket and puts it in a buffer.
     * @param instance of client , buffer , size of data
     * @return bytes of read
     * @warning If the reading operation fails, an error message will be
     * printed, and the client socket will be closed.
     */
    int readData(Client *client, char *buffer, int size);

    /*
     * @brief read one character from client(client choice)
     * @param instance of client
     * @return one characeter
     */
    char readClientChoice(Client *client);

    /*
     * @brief print the input data
     * @param instance of client
     */
    void printClientData(Client *client);

    /*
     *@brief print message(client closed)
     *@param instance of client
     */
    void printClientClose(Client *client);

    /*
     * @brief read size of data from socket
     * @param instance of client
     * @warning this method will write an error message to the client and
     * return -1.
     */
    int readDataSize(Client *client);

    /*
     * @brief close client
     * @param instance of client
     */
    void closeClient(Client *client);

    /*
     * @brief push client to waiting clients
     * @param instance of client
     */
    void pushToQueue(Client *client);

public:
    /*Constructor*/
    Socket(ServerConfiguration &serverConfig)
    {
        this->serverConfig = serverConfig;
	ip = serverConfig.getIp();
	port = serverConfig.getPort();
    }

    /*Destructor*/
    ~Socket()
    {
        for (Client *client : clients)
            delete client;

        clients.clear();
    }
    /**
     *
     * Methods :
     *
     */

    /*@brief close server*/
    void stop();

    /*@brief checks whether socket is running or not*/
    bool isRunning();

    /*
     * @brief Creates a socket for  the server using the provided
     * configuration.
     *
     *
     * @warning this method throws a SocketException if:
     * -There is an error creating the socket.
     * -The socket binding failes.
     * -The listening on the socket failes.
     * -Accepting incoming client connections fails.
     *
     */
    void createSocket();

    /*Getter for sockfd*/
    int getSockfd() const
    {
        return sockfd;
    }

    /*Getter for clients*/
    const std::vector<Client *> &getClients()
    {
        return clients;
    }

    /*Getter for waitingClients */
    std::queue<Client *> &getWaitingClients()
    {
        return waitingClients;
    }

    /*Getter for mutex*/
    std::mutex &getMutex()
    {
        return socketMtx;
    }

    /*Getter for cv*/
    std::condition_variable &getCV()
    {
        return cv;
    }

    /*Getter for serverConfig*/
    ServerConfiguration &getServerConfig()
    {
        return serverConfig;
    }
};
/*
 * To handel socket exceptions
 */
class SocketException : public std::runtime_error
{
public:
    SocketException(const std::string &message) : std::runtime_error(message)
    {
    }
};
#endif

/**
 * \file socket.hpp
 *
 * socket structure.
 *
 * \author MohammadDerhami
 *
 */

#ifndef SOCKET_H
#define SOCKET_H

#include "client.hpp"
#include "config.hpp"

/**
 * @ class socket
 * @ brief this class is responsible for creating  a socket
 */
class Socket
{
public:
    /*
     * @brief Construct a new Socket object.
     * @param object of SeverConfig
     */
    Socket(ServerConfiguration &serverConfig);

    /*
     * @brief Destruct a Socket object
     */
    ~Socket();

    /**
     *
     * Methods :
     *
     */

    /*@brief close server*/
    void stop();

    /*@brief checks whether socket is open or not*/
    bool isOpen();

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

    /*Getters*/
    int getSockfd() const;
    const std::vector<Client *> &getClients();
    std::queue<Client *> &getWaitingClients();
    std::mutex &getMutex();
    std::condition_variable &getCV();

private:
    /* IP */
    std::string ip;

    /* Port */
    int port;

    /*Max clients that can connect*/
    int maxConnection;

    /*Socket discriptor*/
    int sockfd;

    /*Indicates whether the socket is bound to a port.*/
    bool isBound;

    /*Indicates whether the socket is in a listening stat*/
    bool isListening;

    /*Mutex*/
    std::mutex socketMtx;
    std::mutex coutMtx;

    /*Condition variable*/
    std::condition_variable cv;

    /*Number of clients*/
    int clientsNum;

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

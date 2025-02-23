/**
 *
 * \file client.hpp
 *
 * \author : MohammadDerhami
 *
 */

#ifndef CLIENT_H
#define CLIENT_H

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
public:
    /*
     * @brief Construct a new Client object.
     * @param socket , id of client
     */
    Client(int socket, int id);

    /*
     * @brief Destruct a Client object.
     */
    ~Client();

    /*Getters*/
    bool getResultReady() const;
    bool getDataReady() const;
    std::mutex &getMutex();
    std::condition_variable &getCV();
    std::thread &getThread();
    std::thread &getParseThread();
    int getId() const;
    int getClientSocket() const;
    std::string getInputData();
    std::string getResult();

    /*Setters*/
    void setResultReady(bool resultReady);
    void setDataReady(bool dataReady);
    void setThread(std::thread &&thread);
    void setParseThread(std::thread &&thread);
    void setId(int id);
    void setClientSocket(int clientSocket);
    void setInputData(const std::string &inputData);
    void setResult(const std::string &result);

    /* Resets the client state for reuse*/
    void reset();

private:
    int clientSocket;

    std::string inputData;

    int id;

    std::thread clientThread;

    std::thread parseThread;

    std::string result;

    std::mutex clientMtx;

    std::condition_variable cv;

    bool resultReady;

    bool dataReady;
};
#endif

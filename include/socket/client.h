/**
 *
 * \file client.h
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

public:
    Client(int socket, int id) : clientSocket(socket), id(id), resultReady(false), dataReady(false)
    {
    }

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

    /*Setter and Getter for inputData */
    std::string getInputData()
    {
        std::lock_guard<std::mutex> lock(clientMtx);
        return inputData;
    }
    void setInputData(const std::string &inputData)
    {
        std::lock_guard<std::mutex> lock(clientMtx);
        this->inputData = inputData;
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
#endif

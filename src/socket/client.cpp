#include "client.hpp"

/**
 *
 * Implementation for "Client" class
 *
 */

Client::Client(int socket, int id) :
    clientSocket {socket},
    id {id},
    resultReady {false},
    dataReady {false}
{
}
Client::~Client()
{
    if (clientThread.joinable())
        clientThread.join();
    if (parseThread.joinable())
        parseThread.join();
}

/*Resets the client status for reuse by clearing results and data flags.*/
void Client::reset()
{
    std::lock_guard<std::mutex> lock(clientMtx);

    resultReady = false;
    result.clear();

    dataReady = false;
    inputData.clear();
}

bool Client::getResultReady() const
{
    return resultReady;
}
bool Client::getDataReady() const
{
    return dataReady;
}
std::mutex &Client::getMutex()
{
    return clientMtx;
}
std::condition_variable &Client::getCV()
{
    return cv;
}
std::thread &Client::getThread()
{
    return clientThread;
}
std::thread &Client::getParseThread()
{
    return parseThread;
}
int Client::getId() const
{
    return id;
}
int Client::getClientSocket() const
{
    return clientSocket;
}
const std::string& Client::getInputData() 
{
    std::lock_guard<std::mutex> lock(clientMtx);
    return inputData;
}
const std::string& Client::getResult() 
{
    std::lock_guard<std::mutex> lock(clientMtx);
    return result;
}
void Client::setResultReady(bool resultReady)
{
    this->resultReady = resultReady;
}

void Client::setDataReady(bool dataReady)
{
    this->dataReady = dataReady;
}

void Client::setThread(std::thread &&thread)
{
    clientThread = std::move(thread);
}

void Client::setParseThread(std::thread &&thread)
{
    parseThread = std::move(thread);
}

void Client::setId(int id)
{
    this->id = id;
}

void Client::setClientSocket(int clientSocket)
{
    this->clientSocket = clientSocket;
}

void Client::setInputData(const std::string &inputData)
{
    std::lock_guard<std::mutex> lock(clientMtx);
    this->inputData = inputData;
    dataReady = true;
}

void Client::setResult(const std::string &result)
{
    std::lock_guard<std::mutex> lock(clientMtx);
    this->result = result;
    resultReady = true;
}

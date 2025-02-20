/**
 *
 * \ file config.h
 *
 * configuration programm
 *
 * \ author : MohammadDerhami
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <iostream>
//#include <rapidjson/document.h>
//#include <rapidjson/istreamwrapper.h>
#include "nlohmann/json.hpp"
#include <unistd.h>
/**
 * @class ServerConfiguration
 * @brief for configure socket
 */
class ServerConfiguration
{
private:
    int port;
    std::string ip;
    int maxConnection;

public:
    /*Setter and Getter for port */
    int getPort() const
    {
        return port;
    }
    void setPort(int port)
    {
        this->port = port;
    }

    /*Setter and Getter for ipAddress */
    std::string getIp() const
    {
        return ip;
    }
    void setIp(const std::string &ip)
    {
        this->ip = ip;
    }

    /*Setter and Getter for maxConnection*/
    void setMaxConnection(int maxConnection)
    {
        this->maxConnection = maxConnection;
    }
    int getMaxConnection() const
    {
        return maxConnection;
    }
};

/**
 * @class DatabaseConfiguration
 * @brief for configure database
 */
class DatabaseConfiguration
{
private:
    std::string filePath;

public:
    /*Setter and Getter for filePath */
    std::string getFilePath() const
    {
        return filePath;
    }
    void setFilePath(const std::string &filePath)
    {
        this->filePath = filePath;
    }
};

/**
 * @class configuration
 *
 * @brief this class for configuration the programm
 */
class Configuration
{
private:
    DatabaseConfiguration databaseConfig;
    ServerConfiguration serverConfig;

    /**
     * @brief print help of program
     */
    void printHelp();

    /*
     * @brief print steps of implementation of program
     */
    void displayProgramSteps();

    /**
     * @brief print version of program.
     */
    void printVersion();

    /*
     * @brief parse json and configuration program.
     *
     * @param address of config file
     *
     * @warning This method throws a runtime_error if:
     * - The configuration file cannot be opened.
     * - The JSON data cannot be parsed correctly.
     *
     */

    void configurationProgram(const std::string &configFilePath);

    /**
     * @brief parsing command line arguments with getopt
     * and pass config file path to configurationProgram method.
     *
     * @param : argc -> number of arguments , argv -> arguments
     */
    std::string commandLineArgs(int argc, char *argv[]);

public:
    /**
     * @brief config program
     *
     * @param argc , argv
     */
    void config(int argc, char *argv[]);

    /* Getter for DatabaseConfig */
    DatabaseConfiguration &getDatabaseConfig()
    {
        return databaseConfig;
    }
    /* Getter for ServerConfig */
    ServerConfiguration &getServerConfig()
    {
        return serverConfig;
    }
};
#endif

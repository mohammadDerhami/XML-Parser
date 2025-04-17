/**
 *
 * \ file config.hpp
 *
 * configuration programm
 *
 * \ author : MohammadDerhami
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "nlohmann/json.hpp"

#include <fstream>
#include <iostream>
#include <unistd.h>
/**
 * @class ServerConfiguration
 * @brief for configure socket
 */
class ServerConfiguration
{
public:
    /*Getters*/
    int getPort() const;
    int getMaxConnection() const;
    std::string getIp() const;

    /*Setters*/
    void setPort(int port);
    void setIp(const std::string &ip);
    void setMaxConnection(int maxConnection);

private:
    int port;
    std::string ip;
    int maxConnection;
};

/**
 * @class DatabaseConfiguration
 * @brief for configure database
 */
class DatabaseConfiguration
{
public:
    /*Getters*/
    std::string getFilePath() const;

    /*Setters*/
    void setFilePath(const std::string &filePath);

private:
    std::string filePath;
};
/**
 * @class configuration
 *
 * @brief this class for configuration the program
 */
class Configuration
{
public:
    /**
     * @brief config program
     *
     * @param argc , argv
     */
    void config(int argc, char *argv[]);

    /*Getters*/
    DatabaseConfiguration &getDatabaseConfig();

    /*Setters*/
    ServerConfiguration &getServerConfig();

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
};
#endif

/**
 *
 * \ file : config.cpp
 *
 * configuration programm
 *
 * \author : MohammadDerhami
 */

#include "config.h"

/**
 *
 * Implementation for "Configuration" class
 *
 */

/*Configures the application by parsing command-line arguments and loading the configuraion file.*/
void Configuration::config(int argc, char *argv[])
{
    std::string filePath = commandLineArgs(argc, argv);

    configurationProgram(filePath);
}

/*Parses command-line arguments to determine the configuraiton file path.*/
std::string Configuration::commandLineArgs(int argc, char *argv[])
{
    if (argc == 1) {
        std::cerr << "Error : no configuration file path provided \n\n";
        printHelp();
        exit(0);
    }

    int opt;
    std::string filePath;

    bool configFileProvided = false;

    while ((opt = getopt(argc, argv, ":f:vh")) != -1) {
        switch (opt) {
        /* -h -> print help */
        case 'h':
            printHelp();
            break;
        /* -v -> print version */
        case 'v':
            printVersion();
            break;
        /* -f <filePath> */
        case 'f':
            filePath = optarg;
            configFileProvided = true;
            break;
        case ':':
            std::cout << "Error : Option needs a value \n\n";
            printHelp();
            exit(0);
        case '?':
            std::cout << "UnKnown option \n";
            printHelp();
            exit(0);
        }
    }
    /* Handle extra arguments */
    for (int i = optind; i < argc; i++) {
        std::cout << "Non-option argument: " << argv[i] << std::endl;
    }
    /*
     * If the configuration file address is not entered,
     * the program will close
     */
    if (configFileProvided == false) {
        exit(0);
    }
    return filePath;
}

/*Opens the specified configurtion file , parse json and initializes the server and database.*/
void Configuration::configurationProgram(const std::string &configFilePath)
{
    /* open file */
    std::ifstream configFile(configFilePath);
    if (! configFile.is_open()) {
        throw std::runtime_error("Unable to open file: " + configFilePath + "\n");
    }

    /* Create a JSON object from the input file */
    nlohmann::json jsonDocument;
    configFile >> jsonDocument; // Parse the JSON data from the input stream

    /* close file */
    configFile.close();

    /* parse service object */
    if (jsonDocument.contains("servive") && jsonDocument["servive"].is_object()) {
        const auto &service = jsonDocument["servive"];

        if (service.contains("ip") && service["ip"].is_string()) {
            serverConfig.setIp(service["ip"].get<std::string>());
        }
        if (service.contains("port") && service["port"].is_number_integer()) {
            serverConfig.setPort(service["port"].get<int>());
        }
        if (service.contains("maxConnection") && service["maxConnection"].is_number_integer()) {
            serverConfig.setMaxConnection(service["maxConnection"].get<int>());
        }
    }

    /* parse database object */
    if (jsonDocument.contains("database") && jsonDocument["database"].is_object()) {
        const auto &database = jsonDocument["database"];
        if (database.contains("path") && database["path"].is_string()) {
            databaseConfig.setFilePath(database["path"].get<std::string>());
        }
    }

    std::cout << "Program configuraiton was seccussful.\n";
}

/*Prints the help message for command-line usage.*/

void Configuration::printHelp()
{
    std::cout << "Usage:\n"
                 "  ./dbm -f <config file path>            : Use the "
                 "specified configuration file.\n"
                 "  ./dbm -h                                : Display this "
                 "help message.\n"
                 "  ./dbm -v                                : Display the "
                 "current version of the application.\n";
}

/*Prints the current version of the application */

void Configuration::printVersion()
{
    const std::string version = "1.0";
    const std::string releaseDate = "2025-01-10";

    std::cout << "Application Version: " << version << std::endl;
    std::cout << "Release Date: " << releaseDate << std::endl;
}


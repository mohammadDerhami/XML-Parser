/**
 * \ Author : Mohammad Derhami
 *
 * file : parser.hpp
 *
 */

#include "client.hpp"
#include "database.hpp"
#include "tree.hpp"

using namespace Sqlite;
namespace XML
{
/*
 * Class Parser
 *
 * parse XML and pass information to database.
 *
 */
class Parser
{
public:
    /*
     *
     * @brief Process xmlData and store it in database
     *
     * @param pointer to instance of client , pointer to instance of DatabaseManager class
     *
     *	Note: handles exceptions related to XML parsing and database intractions.
     */
    void parseAndStoreXmlData(Client *client, DatabaseManager *database);

private:
    /*
     * @brief Process all nodes recursive and store in database
     * @param pointer to instance of Tree,
     * -pointer to root of tree,
     * -uuid
     * -name of main table
     * -pointer to instance of DatabaseManager
     */
    void storeXmlNodesInDatabase(Tree *tree, Node *root, const std::string &uuid,
                                 const std::string &mainTable, DatabaseManager *database);

    /*
     * @brief Inserts a new entry into the database table using the provided
     * uuid ,property names and values from the given node.
     *
     * @param
     * -pointer to an instance of DatabaseManager class
     * -uuid The unique id
     * -names a refrence of vector containing the names of properties
     * -names a refrence of vector containing the values of properties
     */
    void insertIntoDatabase(DatabaseManager *database, const std::string &uuid,
                            std::vector<std::string> &names, std::vector<std::string> &values,
                            const std::string &tableName);

    /*
     * @brief Create a new table in the database
     *
     * @param
     * -pointer to an instance of DatabaseManager class
     * -the name of the main table
     * -a vector containing the name of the properties for the table
     */
    void createTableIntoDatabase(DatabaseManager *database, const std::string &mainTable,
                                 const std::string &tableName, std::vector<std::string> &names);

    /*
     * @brief handle exception
     * @param pointer to instance of client
     * -exception
     * -pointer to instance of Tree
     */
    void handleException(Client *client, const std::exception &e, Tree *tree);
};

} /* namespace XML */

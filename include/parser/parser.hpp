/**
 * \ Author : Mohammad Derhami
 *
 * file : parser.hpp
 *
 */

#include "client.hpp"
#include "database.hpp"
#include "tree.hpp"

using namespace SQLite;
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
     * @brief handle exception
     * @param pointer to instance of client
     * -exception
     * -pointer to instance of Tree
     */
    void handleException(Client *client, const std::exception &e, Tree *tree);
};

} /* namespace XML */

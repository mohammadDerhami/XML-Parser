#include "parser.hpp"

namespace XML
{

/**
 *
 * Implementation for "Parser" Class
 *
 */

/*
 * -Process XML data received from a client and either retrieves data from the database(for SELECT
 * queries)
 * -or stores the XML data into the database(for INSERT operation)
 * -handles exceptions related to XML parsing and database intractions.
 */
void Parser::parseAndStoreXmlData(Client *client, DatabaseManager *database)
{
    Tree *tree;
    try {
        std::string xmlData = client->getInputData();

        /*Builds the tree*/
        tree = new Tree {xmlData};

        if (tree->getIsSelectType()) {
            if (! tree->getTableName().empty())
                client->setResult(database->fetchTableDataAsXML(tree->getTableName()));
            else
                client->setResult(database->fetchAllTablesAsXML());

            delete tree;
            tree = nullptr;

            client->getCV().notify_one();
        } else {
            Node *root = tree->getRoot();

            const std::string uuid = tree->getUuid();
            const std::string mainTable = tree->getMainTable();

            storeXmlNodesInDatabase(tree, root, uuid, mainTable, database);

            client->setResult("done :) \n");

            delete tree;
            tree = nullptr;

            client->getCV().notify_one();
        }

    } catch (const ParseXmlException &pe) {
        handleException(client, pe, tree);
    } catch (const DatabaseException &de) {
        handleException(client, de, tree);
    } catch (const std::exception &e) {
        handleException(client, e, tree);
    }
}

/*Handles exceptions that occur during XML parsing or database operation*/
void Parser::handleException(Client *client, const std::exception &e, Tree *tree)
{
    if (tree) {
        delete tree;
        tree = nullptr;
    }
    std::string message = "Error : " + std::string(e.what());
    client->setResult(message);
    client->getCV().notify_one();
}

/*
 * Stores XML nodes into a specified database by Non-recursively traversing the XML tree.
 *
 * This method process each node in the XML tree ( of the Tree object ).
 * For each node:
 * 1.It checks if the node is Element node.
 * 2.If the node has property nodes,it collects their names and values.
 * 3.The method checks if a table exist in the database:
 * -If it exists ,it inserts the collected properties and their values into the database.
 * -If it does not exits ,it first create a new table then inserts the values.
 *
 * 4.The method recursively processes child nodes and sibling nodes to ensure all nodes in the
 * XML structure are stored in the database.
 */

void Parser::storeXmlNodesInDatabase(Tree *tree, Node *root, const std::string &uuid,
                                     const std::string &mainTable, DatabaseManager *database)
{
    if (! root)
        return;

    std::stack<Node *> nodeStack;
    nodeStack.push(root);

    while (! nodeStack.empty()) {
        Node *node = nodeStack.top();
        nodeStack.pop();

        if (node->isElementNode() && node->hasPropertyNode()) {
            std::string nodeName = node->getName();

            std::vector<std::string> names = node->collectPropertyNames();
            std::vector<std::string> values = node->collectPropertyValues();

            if (database->isExistTable(node->getName())) {
                database->insertIntoTable(uuid, names, values, nodeName);
            } else {
                bool isMainTable = (mainTable == nodeName);
                database->createTable(nodeName, names, isMainTable, mainTable);

                database->insertIntoTable(uuid, names, values, nodeName);
            }
        }

        Node *next = node->getNext();
        if (next) {
            nodeStack.push(next);
        }

        Node *child = node->getChild();
        if (child) {
            nodeStack.push(child);
        }
    }
}
/* Stores XML nodes into a specified database by Non-recursively traversing the XML tree.*/
/*void Parser::storeXmlNodesInDatabase(Tree *tree, Node *current, const std::string &uuid,
                                     const std::string &mainTable, DatabaseManager *database)
{
    while (current) {
        if (! current)
            break;
        if (current->isElementNode() && current->hasPropertyNode()) {
            std::string nodeName = current->getName();

            std::vector<std::string> names = current->collectPropertyNames();
            std::vector<std::string> values = current->collectPropertyValues();

            if (database->isExistTable(current->getName())) {
                insertIntoDatabase(database, uuid, names, values, nodeName);
            } else {
                createTableIntoDatabase(database, mainTable, nodeName, names);
                insertIntoDatabase(database, uuid, names, values, nodeName);
            }
        }
        Node *child = current->getChild();
        if (child)
            storeXmlNodesInDatabase(tree, child, uuid, mainTable, database);

        Node *next = current->getNext();
        current = next;
    }
}*/

} /*namespace XML*/

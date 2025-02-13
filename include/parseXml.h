/**
 * \ Author : Mohammad Derhami
 *
 * file : parse xml
 *
 */

#include "database.h"
#include "server.h"

#include <cstring>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <stack>
#include <vector>

/*
 * Class Node
 * @brief:
 * Type of nodes :
 * 1.Element node : defined in xmlNode structure.
 * 2.Text node : defined in xmlNode structure.
 * 3.Object node : A node that has a child elementNode.
 * 4.Property node : A node that is not a object node.
 *
 */
class Node
{
public:
    /*Constructor*/
    Node(xmlNodePtr xmlNode) : xmlNode_(xmlNode)
    {
    }

    /*Destructor*/
    ~Node()
    {
        delete parentNode;
        delete nextNode;
        delete childNode;
    }

    /* @brief Checks whether the node is an object node or not */
    bool isObjectNode();

    /* @brief Checks whether the node is an element node or not */
    bool isElementNode();

    /* @brief Checks whether the node is a property node or not */
    bool isPropertyNode();

    /* @brief Checks whether the node has a property node or not */
    bool hasPropertyNode();

    /*
     * @brief Returns the content of the node
     * @retunr content of node
     */
    std::string getContent();

    /*
     * @brief Returns the name of the node
     * @return name of node
     */
    std::string getName();

    /*
     * @brief Fills a vector with the names of properties of the current
     * node
     * @param names a refrence to a vector where property names will be
     * stored.
     */
    void propertyNames(std::vector<std::string> &names);

    /*
     * @brief Fills a vector with the values of properties of the current
     * node
     * @param names a refrence to a vector where property values will be
     * stored.
     */
    void propertyValues(std::vector<std::string> &values);

    /*
     * @brief Returns child of the node
     * @return pointer from the child of the node
     */
    Node *getChild();

    /*
     * @brief Returns next node
     * @return pointer from the next of the node
     */
    Node *getNext();

    /*
     * @brief Returns parent node
     * @return pointer from the parent of the node.
     */
    Node *getParent();

    /* @brief Getter for xmlNode_ */
    xmlNodePtr getXmlNode() const
    {
        return xmlNode_;
    }

    /* @brief Setter for xmlNode_ */
    void setXmlNode(xmlNodePtr xmlNode_)
    {
        this->xmlNode_ = xmlNode_;
    }

private:
    Node *parentNode = nullptr;
    Node *childNode = nullptr;
    Node *nextNode = nullptr;

    xmlNodePtr xmlNode_;

    /*
     * @brief Converts xmlChar to string
     * @param xml char that converts to string
     * @return string
     */
    std::string xmlCharToString(const xmlChar *xml);
};

/*
 * Class Document of XML
 */
class Document
{
public:
    /*Constructor*/
    Document(const std::string &xmlData)
    {
        this->xmlData = xmlData;
        initialize();
    }

    /*Destructor*/
    ~Document()
    {
        delete root;
        if (doc_)
            xmlFreeDoc(doc_);
        xmlCleanupParser();
    }
    /*Getter for isSelectType*/
    bool getIsSelectType() const
    {
        return isSelectType;
    }
    /*Getter for root*/
    Node *getRoot()
    {
        return root;
    }
    /* Getter for uuid*/
    std::string getUuid() const
    {
        return uuid;
    }
    /*Getter for mainTable*/
    std::string getMainTable() const
    {
        return mainTable;
    }

private:
    xmlDocPtr doc_ = nullptr;
    Node *root = nullptr;
    std::string uuid;
    std::string xmlData;
    std::string mainTable;
    bool isSelectType;

    /*
     *
     * Methods
     *
     */

    /*
     * @brief initialize variables
     *
     * @warning This method throws ParseXmlExcepion if:
     * -if the doc is null.
     * -if the xmlRoot is null.
     * -if the uuid not found.
     *
     */
    void initialize();

    /* @brief Finds uuid and values the mainTable */
    void findUuid();

    /* @brief determine the input type */
    void determineType();
};

/*
 * Class XmlParser
 *
 * parse XML and pass information to database.
 *
 */
class XmlParser
{
public:
    /*
     *
     * @brief Process xmlData and store it in database
     *
     * @param instance of client , instance of DatabaseManager class
     *
     *
     */
    void parseAndStoreXmlData(Client *client, DatabaseManager *database);

private:
    /*
     * @brief Process all nodes recursive and store in database
     * @param instance of Document , instance of DatabaseManager
     */
    void storeXmlNodesInDatabase(Document &doc, DatabaseManager *database);

    /*
     * @brief Inserts a new entry into the database table using the provided
     * uuid ,property names and values from the given node.
     *
     * @param
     * -pointer to an instance of DatabaseManager class
     * -uuid The unique id
     * -names a refrence of vector containing the names of properties
     * -names a refrence of vector containing the values of properties
     * -a pointer to the  node
     */
    void insertIntoDatabase(DatabaseManager *database, const std::string &uuid,
                            std::vector<std::string> &names, std::vector<std::string> &values,
                            Node *node);

    /*
     * @brief Create a new table in the database
     *
     * @param
     * -pointer to an instance of DatabaseManager class
     * -the name of the main table
     * -a pointer to the node
     * -a vector containing the name of the properties for the table
     */
    void createTableIntoDatabase(DatabaseManager *database, const std::string &mainTable,
                                 Node *node, std::vector<std::string> &names);

    /*
     * @brief handle exception
     * @param client , exception
     */
    void handleException(Client *client, const std::exception &e);
};
/*
 * Class ParseXmlException
 * Inheritance from runtime_error class
 *
 * To handle parse errors
 */
class ParseXmlException : public std::runtime_error
{
public:
    ParseXmlException(const std::string &message) : std::runtime_error(message)
    {
    }
};


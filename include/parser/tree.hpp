/**
 *
 * \file : tree.hpp
 *
 * \author : MohammadDerhami
 *
 *
 */

#ifndef TREE_H
#define TREE_H

#include <cstring>
#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <memory>
#include <stack>
#include <unordered_map>
#include <vector>

namespace XML
{

/*
 * Class Node
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
    /*
     * @brief Construct a new Node object.
     * @param object of XmlNodePtr
     */
    Node(xmlNodePtr node);

    /*@brief  Checks if the current node is an object node(has child element)*/
    bool isObjectNode();

    /*@brief Checks if the current node is an element node*/
    bool isElementNode();

    /*@brief Checks if the current node ia a property node(not an object)*/
    bool isPropertyNode();

    /*@brief  Checks if the current node has any property nodes*/
    bool hasPropertyNode();

    /* @brief Returns name of node.*/
    std::string getName();

    /* @brief Returns content of node*/
    std::string getContent();
    /*@brief Return first child */
    Node *getChild() const;

    /*
     * @brief Converts xmlChar to string
     * @param xml char that converts to string
     * @return string
     */
    std::string xmlCharToString(const xmlChar *xml);

    /*
     * @brief retrieves the names of all properties
     *
     * @return vector to store names of all properties of node
     */
    std::vector<std::string> collectPropertyNames();

    /*
     * @brief retrieves the values of all properties
     *
     * @return vector to store values of all properties of node.
     */
    std::vector<std::string> collectPropertyValues();

    /*
     * @brief Adds child
     *
     * @param child node
     */
    void addChild(Node *child);
    /*Setters*/
    void setNext(Node *next);
    void setParent(Node *parent);

    /*Getters*/
    std::vector<Node *> &getChildren();
    xmlNodePtr getXmlNode() const;
    Node *getNext() const;
    Node *getParent() const;

private:
    xmlNodePtr xmlNode;
    Node *parent;
    std::vector<Node *> children;
    Node *next;
};
/**
 * class Tree of XML
 */
class Tree
{
public:
    /*
     * @brief Construct a new Tree object.
     * @param xmlData
     */
    Tree(std::string &xmlData);

    /*
     * @brief Destruct a Tree object
     */
    ~Tree();

    /*
     * @brief recursively searches for a node with the given name.
     * @param name of node.
     * @return a pointer to the node with the matching name.
     */
    Node *find(const std::string &name);

    /*Getters*/
    bool getIsSelectType() const;
    const std::string &getUuid() const;
    const std::string &getMainTable() const;
    Node *getRoot();
    const std::string &getTableName() const;

private:
    xmlDocPtr xmlDoc;

    std::vector<Node *> allNodes;

    Node *root;

    std::string uuid;

    std::string mainTable;

    std::string xmlData;

    bool isSelectType;

    std::string tableName;

    /*
     *
     *
     * Methods
     *
     *
     */

    /*
     * @brief This method reads XML data from the `xmlData` member variable,
     * parses it using libxml2,
     * constructs the tree structure,
     * determines the type of the XML document(SELECT OR INSERT),
     * retrieves the UUID if it exists,
     * and identifies the main table name
     *
     * @warning
     * throws ParseXmlException if:
     * -UUID node is not found
     * -parent of UUID node is not found
     */
    void initialize();

    /*
     * @brief Non-Recursively builds a tree structure from an XML node.
     *
     * @param
     * -xmlNode a pointer to the libxml2 XML node to process.
     *
     * @return a pointer to the newly created node.
     */
    Node *buildTree(xmlNodePtr xmlNode);

    /*
     * @brief Determines the type of the XML operation(select or insert).
     * Updates the isSelectType.
     */
    void determineType();

    /*
     * @brief Non-Recursively searches for a node with the given name
     * @param node->root , name of node
     * @return a pointer to the node with the matching name
     */
    Node *findNode(Node *root, const std::string &name);

    /* @brief frees all memory allocated for the nodes in the tree */
    void freeTree();
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

} /*namespace XML*/
#endif

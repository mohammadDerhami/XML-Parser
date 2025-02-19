/**
 *
 * \file : tree.h
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
#include <vector>

namespace XML
{

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
private:
    xmlNodePtr xmlNode;
    Node *parent;
    std::vector<Node *> children;
    Node *next;

public:
    /*Constructor*/
    Node(xmlNodePtr node) : xmlNode(node), parent(nullptr), next(nullptr)
    {
    }

    /*@brief  Checks if the current node is an object node(has child element)*/
    bool isObjectNode();

    /*@brief Checks if the current node is an element node*/
    bool isElementNode();

    /*@brief Checks if the current node ia a property node(not an object)*/
    bool isPropertyNode();

    /*@brief  Checks if the current node has any property nodes*/
    bool hasPropertyNode();

    /* @brief Return name of node.*/
    std::string getName();

    /* @brief Return content of node*/
    std::string getContent();

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

    /*Setter and Getter for next node*/
    void setNext(Node *next)
    {
        this->next = next;
    }
    Node *getNext() const
    {
        return next;
    }

    /*Setter and Getter for parent node */
    void setParent(Node *parent)
    {
        this->parent = parent;
    }
    Node *getParent() const
    {
        return parent;
    }
    /* Return first child */
    Node *getChild() const
    {
        if (! children.empty())
            return children.front();
        else
            return nullptr;
    }

    /*Setter and Getter for vector to store children nodes*/
    std::vector<Node *> &getChildren()
    {
        return children;
    }
    xmlNodePtr getXmlNode() const
    {
        return xmlNode;
    }
};
/**
 * class Tree of XML
 */
class Tree
{
private:
    xmlDocPtr xmlDoc;

    std::vector<Node *> allNodes;

    Node *root;

    std::string uuid;

    std::string mainTable;

    std::string xmlData;

    bool isSelectType;

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
     * @brief recursively builds a tree structure from an XML node.
     *
     * @param
     * -xmlNode a pointer to the libxml2 XML node to process.
     * -parent a pointer to the parent node.
     *
     * @return a pointer to the newly created node.
     */
    Node *buildTree(xmlNodePtr xmlNode, Node *parent = nullptr);

    /*
     * @brief Determines the type of the XML operation(select or insert).
     * Updates the isSelectType.
     */
    void determineType();

    /*
     * @brief Recursively searches for a node with the given name
     * @param node->root , name of node
     * @return a pointer to the node with the matching name
     */
    Node *findNode(Node *node, const std::string &name);

    /* @brief frees all memory allocated for the nodes in the tree */
    void freeTree();

public:
    /*Constructor*/
    Tree(std::string &xmlData)
    {
        this->xmlData = xmlData;
        initialize();
    }
    /*Destructor*/
    ~Tree()
    {
        freeTree();
        xmlFreeDoc(xmlDoc);
    }

    /*
     * @brief recursively searches for a node with the given name.
     * @param name of node.
     * @return a pointer to the node with the matching name.
     */
    Node *find(const std::string &name);

    /*Getter for isSelectType */
    bool getIsSelectType() const
    {
        return isSelectType;
    }

    /*Getter for uuid */
    std::string getUuid() const
    {
        return uuid;
    }

    /*Getter for main table */
    std::string getMainTable() const
    {
        return mainTable;
    }

    /*Getter for root*/
    Node *getRoot()
    {
        return root;
    }
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

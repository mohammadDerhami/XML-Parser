#include "tree.h"
namespace XML
{

/**
 *
 * Implementation for "Node" class
 *
 */

/* Checks if the current node is an object node(has child element)*/
bool Node::isObjectNode()
{
    for (Node *child : children) {
        if (child->isElementNode())
            return true;
    }
    return false;
}

/* Checks if the current node is an element node*/
bool Node::isElementNode()
{
    return xmlNode->type == XML_ELEMENT_NODE;
}

/* Checks if the current node ia a property node(not an object)*/
bool Node::isPropertyNode()
{
    return ! this->isObjectNode();
}

/* Checks if the current node has any property nodes*/
bool Node::hasPropertyNode()
{
    for (Node *child : children) {
        if (child->isPropertyNode() && child->isElementNode())
            return true;
    }
    return false;
}

/* Converts an xmlChar object to string */
std::string Node::xmlCharToString(const xmlChar *xml)
{
    return std::string(reinterpret_cast<const char *>(xml));
}

/* Returns the content of the current node */
std::string Node::getContent()
{
    return xmlCharToString(xmlNodeGetContent(xmlNode));
}

/* Returns the name of the current node */
std::string Node::getName()
{
    return xmlCharToString(xmlNode->name);
}

/* Fills a vector with the property names of the current node */
std::vector<std::string> Node::collectPropertyNames()
{
    std::vector<std::string> propertyNames;
    for (Node *child : children) {
        if (child->isPropertyNode() && child->isElementNode()) {
            if (child->getName() == "uuid")
                continue;
            propertyNames.push_back(child->getName());
        }
    }

    return propertyNames;
}

/* Fills a vector with the property valuse of the current node*/
std::vector<std::string> Node::collectPropertyValues()
{
    std::vector<std::string> propertyValues;
    for (Node *child : children) {
        if (child->isPropertyNode() && child->isElementNode()) {
            if (child->getName() == "uuid")
                continue;
            propertyValues.push_back(child->getContent());
        }
    }
    return propertyValues;
}

/*
 *
 *
 * Implementation for "Tree" Class
 *
 *
 */

/*
 * Initializes the Tree object by parsing the provided XML data.
 *
 * throws an exception if:
 * -the document is invalid or empty
 * -uuid is not found
 * -parent of uuid is not founc
 */
void Tree::initialize()
{
    xmlDoc = xmlReadMemory(xmlData.c_str(), xmlData.length(), nullptr, nullptr, 0);
    if (xmlDoc == nullptr) {
        throw ParseXmlException("Error parsing XML data: Failed to parse XML "
                                "document!!!\n");
    }

    root = buildTree(xmlDocGetRootElement(xmlDoc));

    determineType();

    if (! isSelectType) {
        Node *uuidNode = find("uuid");

        if (! uuidNode)
            throw ParseXmlException("Uuid node not found!!!");

        uuid = uuidNode->getContent();

        Node *parentNode = uuidNode->getParent();
        if (! parentNode)
            throw ParseXmlException("Parent of uuid node not found!!!");

        mainTable = parentNode->getName();

        if (uuid.empty())
            throw ParseXmlException("Uuid not found!!!\n");
    }
}

/*
 * Recursively builds a tree structure from an XML node.
 *
 * Note:
 * Recursively builds a tree structure from an XML node.
 * This function operates in a depth-first manner, traversing the XML document
 * and creating corresponding Node objects for each XML element node encountered.
 *
 * Key Points:
 * - Recursively constructs the tree by creating Node objects for each XML element node.
 * - Uses depth-first traversal to process child nodes.
 * - Maintains parent-child relationships and links sibling nodes using the `next` pointer.
 * - Tracks all created nodes in the `allNodes` list.
 *
 * Parameters:
 * - xmlNode: The current XML node to process.
 * - parent: The parent Node of the current XML node.
 *
 * Returns:
 * - The root node of the subtree constructed from the given XML node.
 *
 */
Node *Tree::buildTree(xmlNodePtr xmlNode, Node *parent)
{
    if (! xmlNode)
        return nullptr;

    Node *node = new Node(xmlNode);
    allNodes.push_back(node);
    node->setParent(parent);

    /*lastChild node (next of child node)*/
    Node *lastChild = nullptr;
    for (xmlNodePtr cur = xmlNode->children; cur; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE) {
            Node *child = buildTree(cur, node);
            if (child)
                node->getChildren().push_back(child);
            if (lastChild)
                lastChild->setNext(child);
            lastChild = child;
        }
    }
    return node;
}

/*
 *Determines the type of the XML operation(select or insert).
 *Updates the isSelectType.
 */
void Tree::determineType()
{
    std::string operationType;

    for (Node *node : root->getChildren()) {
        if (node->isElementNode() && strcmp(node->getName().c_str(), "operation") == 0) {
            xmlChar *type = xmlGetProp(node->getXmlNode(), BAD_CAST "type");
            if (type != nullptr) {
                operationType = reinterpret_cast<const char *>(type);
                xmlFree(type);
            }
            break;
        }
    }

    if (strcmp(operationType.c_str(), "select") == 0)
        isSelectType = true;
    else
        isSelectType = false;
}

/*Pass name and root to the findNode method*/
Node *Tree::find(const std::string &name)
{
    return findNode(root, name);
}

/*Recursively searches for a node with the given name.*/
Node *Tree::findNode(Node *node, const std::string &name)
{
    if (! node)
        return nullptr;

    if (node->isElementNode() && node->getName() == name) {
        return node;
    }

    Node *child = node->getChild();
    while (child) {
        Node *found = findNode(child, name);
        if (found) {
            return found;
        }
        child = child->getNext();
    }

    return nullptr;
}

/*Frees all memory allocated for the nodes in the tree */
void Tree::freeTree()
{
    for (Node *node : allNodes) {
        if (node)
            delete node;
    }
    allNodes.clear();
}
} /*namespace XML*/


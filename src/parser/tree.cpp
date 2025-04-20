#include "tree.hpp"
namespace XML
{

/**
 *
 * Implementation for "Node" class
 *
 */

Node::Node(xmlNodePtr node) : xmlNode {node}, parent {nullptr}, next {nullptr}
{
}

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

/* Returns first child*/
Node *Node::getChild() const
{
    if (! children.empty())
        return children.front();
    else
        return nullptr;
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

/* Adds child*/
void Node::addChild(Node *child)
{
    children.push_back(child);
}

void Node::setNext(Node *next)
{
    this->next = next;
}
void Node::setParent(Node *parent)
{
    this->parent = parent;
}
std::vector<Node *> &Node::getChildren()
{
    return children;
}

xmlNodePtr Node::getXmlNode() const
{
    return xmlNode;
}
Node *Node::getNext() const
{
    return next;
}
Node *Node::getParent() const
{
    return parent;
}

/*
 *
 *
 * Implementation for "Tree" Class
 *
 *
 */

Tree::Tree(std::string &xmlData)
{
    this->xmlData = xmlData;
    initialize();
}
Tree::~Tree()
{
    freeTree();
    xmlFreeDoc(xmlDoc);
}

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
    if (xmlDoc == nullptr)
        throw ParseXmlException("Failed to parse XML document!!!\n");

    /*Builds tree*/
    root = buildTree(xmlDocGetRootElement(xmlDoc));

    /*Determines xmldata type*/
    determineType();

    if (! isSelectType) {
        /*Finds UUID node*/
        Node *uuidNode = find("uuid");

        if (! uuidNode)
            throw ParseXmlException("Uuid node not found!!!\n");

        uuid = uuidNode->getContent();

        Node *parentNode = uuidNode->getParent();
        if (! parentNode)
            throw ParseXmlException("Parent of uuid node not found!!!\n");

        mainTable = parentNode->getName();

        if (uuid.empty())
            throw ParseXmlException("Uuid not found!!!\n");
    }
}
/*
 * Non-Recursively builds a tree structure from an XML node.
 *
 * This method constructs a tree of Node objects corresponding to the
 * XML structure starting from the provided xmlNode. It performs a depth-first
 * traversal of the XML document, creating Node objects for each XML
 * element node encountered, establishing parent-child relationships,
 * and linking sibling nodes.
 *
 * Key Points:
 * - Constructs the tree non-recursively by utilizing a stack.
 * - Processes each XML element node and its children in depth-first order.
 * - Maintains parent-child relationships and sets sibling links using the `next` pointer.
 * - Tracks all created Node objects in the `allNodes` list.
 *
 * Parameters:
 * - xmlNode: A pointer to the current XML node being processed.
 *
 * Returns:
 * - A pointer to the root Node of the constructed tree, representing
 *   the subtree derived from the provided XML node.
 */
Node *Tree::buildTree(xmlNodePtr xmlNode)
{
    if (! xmlNode)
        return nullptr;

    /*Create a stack to hold pairs of xmlNodePtr and Node*/
    std::stack<std::pair<xmlNodePtr, Node *>> nodeStack;
    Node *root = new Node(xmlNode);
    allNodes.push_back(root);
    nodeStack.push({xmlNode, root});

    /*While there are still nodes to process in the stack*/
    while (! nodeStack.empty()) {
        std::pair<xmlNodePtr, Node *> nodeInfo = nodeStack.top();
        xmlNodePtr currentXmlNode = nodeInfo.first;
        Node *currentNode = nodeInfo.second;
        nodeStack.pop();

        Node *lastChild = nullptr;

        /*Iterate through the children of the current XML node*/
        for (xmlNodePtr cur = currentXmlNode->children; cur; cur = cur->next) {
            if (cur->type == XML_ELEMENT_NODE) {
                Node *child = new Node(cur);
                allNodes.push_back(child);
                child->setParent(currentNode);
                currentNode->addChild(child);

                /*Set the next pointer of the last child if it exists*/
                if (lastChild) {
                    lastChild->setNext(child);
                }
                lastChild = child;

                /*Push the current xmlNode and the newly created child Node onto the stack*/
                nodeStack.push({cur, child});
            }
        }
    }
    return root;
}

/*Recursively builds a tree structure from an XML node.*/
/*Node *Tree::buildTree(xmlNodePtr xmlNode, Node *parent)
{
    if (! xmlNode)
        return nullptr;

    Node *node = new Node(xmlNode);
    allNodes.push_back(node);
    node->setParent(parent);

    Node *lastChild {nullptr};
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
}*/

/*
 *Determines the type of the XML operation(select or insert).
 *Updates the isSelectType.
 *
 *Throws an exception if :
 *-type attribute is nall.
 */
void Tree::determineType()
{
    std::string operationType;
    std::string tableName;

    for (Node *node : root->getChildren()) {
        if (node->isElementNode() && strcmp(node->getName().c_str(), "operation") == 0) {
            xmlChar *type = xmlGetProp(node->getXmlNode(), BAD_CAST "type");
            if (type != nullptr) {
                operationType = reinterpret_cast<const char *>(type);
                xmlFree(type);
            } else
                throw ParseXmlException("type attribute is null in <operation> element\n");

            for (Node *childNode : node->getChildren()) {
                if (childNode->isElementNode() &&
                    strcmp(childNode->getName().c_str(), "table") == 0) {
                    tableName = childNode->getContent();
                    break;
                }
            }

            break;
        }
    }

    if (strcmp(operationType.c_str(), "select") == 0) {
        isSelectType = true;
        this->tableName = tableName;
    } else
        isSelectType = false;
}

/*Pass name and root to the findNode method*/
Node *Tree::find(const std::string &name)
{
    return findNode(root, name);
}

/* Non-recursive search for a node with the given name */
Node *Tree::findNode(Node *root, const std::string &name)
{
    if (! root)
        return nullptr;

    /*Create a stack to hold nodes during traversal*/
    std::stack<Node *> nodeStack;
    nodeStack.push(root);

    /*While there are still nodes to process in the stack*/
    while (! nodeStack.empty()) {
        Node *currentNode = nodeStack.top();
        nodeStack.pop();

        /*Check if the current node is an element node and matches the name*/
        if (currentNode->isElementNode() && currentNode->getName() == name)
            return currentNode;

        Node *child = currentNode->getChild();
        while (child) {
            nodeStack.push(child);
            child = child->getNext();
        }
    }
    return nullptr;
}

/*Recursively searches for a node with the given name.*/
/*Node *Tree::findNode(Node *node, const std::string &name)
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
}*/

/*Frees all memory allocated for the nodes in the tree */
void Tree::freeTree()
{
    for (Node *node : allNodes) {
        if (node) {
            delete node;
            node = nullptr;
        }
    }
    allNodes.clear();
}
bool Tree::getIsSelectType() const
{
    return isSelectType;
}
const std::string &Tree::getUuid() const
{
    return uuid;
}
const std::string &Tree::getMainTable() const
{
    return mainTable;
}
Node *Tree::getRoot()
{
    return root;
}
const std::string &Tree::getTableName() const
{
    return tableName;
}

} /*namespace XML*/

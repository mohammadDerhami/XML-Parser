#include "../include/parseXml.h"
/**
 *
 * Implementation for "XmlParser" Class
 *
 */

/*Process xml data received from a client and stores it in the database */
void XmlParser::parseAndStoreXmlData(Client *client, DatabaseManager *database)
{
    try {
        std::string xmlData = client->getXmlData();
        Document doc(xmlData);

        if (doc.getIsSelectType()) {
            client->setResult(database->fetchAllTablesAsXML());
            client->getCV().notify_one();
        } else {
            storeXmlNodesInDatabase(doc, database);
            client->setResult("done :) \n");
            client->getCV().notify_one();
        }

    } catch (const ParseXmlException &pe) {
        handleException(client, pe);
    } catch (const DatabaseException &de) {
        handleException(client, de);
    } catch (const std::exception &e) {
        handleException(client, e);
    }
}

/*Handles exceptions that occur during XML parsing or database operation*/
void XmlParser::handleException(Client *client, const std::exception &e)
{
    std::string message = "Error : " + std::string(e.what());
    client->setResult(message);
    client->getCV().notify_one();
}

/*
 *Traverses the XML document's nodes and stores them in the database.
 *Utilizes stack for backtracking through child nodes.
 */
void XmlParser::storeXmlNodesInDatabase(Document &doc, DatabaseManager *database)
{
    /* Vector to store the names of each property of the node */
    std::vector<std::string> names;

    /* Vector to store the values of each property of the node */
    std::vector<std::string> values;

    const std::string uuid = doc.getUuid();
    const std::string mainTable = doc.getMainTable();
    Node *current = doc.getRoot();

    std::stack<Node *> nodeStack;

    while (current || ! nodeStack.empty()) {
        while (current) {
            nodeStack.push(current);
            current = current->getChild(doc);
        }

        current = nodeStack.top();
        nodeStack.pop();

        if (current->isElementNode()) {
            if (current->hasPropertyNode(doc) && current->isObjectNode(doc)) {
                current->propertyNames(names, doc);
                current->propertyValues(values, doc);

                if (database->isExistTable(current->getName()))

                {
                    insertIntoDatabase(database, uuid, names, values, current);
                } else {
                    createTableIntoDatabase(database, mainTable, current, names);
                    insertIntoDatabase(database, uuid, names, values, current);
                }
            }
        }

        current = current->getNext(doc);
    }
}

/* Inserts data into the specified table in the database*/
void XmlParser::insertIntoDatabase(DatabaseManager *database, const std::string &uuid,
                                   std::vector<std::string> &names,
                                   std::vector<std::string> &values, Node *node)
{
    database->insertIntoTable(uuid, names, values, node->getName());
}

/* Create a new table in the database based on node properties */
void XmlParser::createTableIntoDatabase(DatabaseManager *database, const std::string &mainTable,
                                        Node *node, std::vector<std::string> &names)
{
    bool isMainTable = (mainTable == node->getName());
    database->createTable(node->getName(), names, isMainTable, mainTable);
}
/**
 *
 *
 * Implementation for "Document" Class
 *
 *
 */

/*
 * Initializes the Document object by parsing the provided XML data.
 * throws an exception if the document is invalid or empty
 */
void Document::initialize()
{
    doc_ = xmlReadMemory(xmlData.c_str(), xmlData.length(), nullptr, nullptr, 0);
    if (doc_ == nullptr) {
        throw ParseXmlException("Error parsing XML data: Failed to parse XML "
                                "document!!!\n");
    }

    xmlNodePtr xmlRoot = xmlDocGetRootElement(doc_);
    if (xmlRoot == nullptr) {
        throw ParseXmlException("Empty xml document!!!\n");
    }

    root = createNode(xmlRoot);
    determineType();

    if (! isSelectType) {
        findUuid();

        if (uuid.empty())
            throw ParseXmlException("Uuid not found!!!\n");
    }
}

/*
 *Determines the type of the XML operation(select or insert).
 *Updates the isSelectType.
 */
void Document::determineType()
{
    std::string operationType;

    for (Node *node = root->getChild(*this); node; node = node->getNext(*this)) {
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

/*
 * Finds the UUID and the main table name from the XML nodes.
 * Throws an exception if the UUID is not found.
 */
void Document::findUuid()
{
    Node *current = root;
    std::stack<Node *> nodeStack;

    while (current || ! nodeStack.empty()) {
        while (current) {
            nodeStack.push(current);
            current = current->getChild(*this);
        }

        current = nodeStack.top();
        nodeStack.pop();

        if (strcmp(current->getName().c_str(), "uuid") == 0) {
            uuid = current->getContent();
            mainTable = current->getParent(*this)->getName();
            return;
        }

        current = current->getNext(*this);
    }
}
/*
 * Creates a new node object from the given XML node pointer.
 * Adds the created node to the vector.
 */
Node *Document::createNode(xmlNodePtr xmlNode)
{
    Node *node = new Node(xmlNode);
    allNodes.push_back(node);
    return node;
}
/**
 *
 * Implementation for "Node" class
 *
 */

/* Returns the child of the current node */
Node *Node::getChild(Document &doc)
{
    if (xmlNode_->children) {
        child = doc.createNode(xmlNode_->children);
        return child;
    }
    return nullptr;
}

/* Returns the next node of the current node */
Node *Node::getNext(Document &doc)
{
    if (xmlNode_->next) {
        next = doc.createNode(xmlNode_->next);
        return next;
    }
    return nullptr;
}

/* Returns the parent node of the current node */
Node *Node::getParent(Document &doc)
{
    if (xmlNode_->parent) {
        parent = doc.createNode(xmlNode_->parent);
        return parent;
    }
    return nullptr;
}

/* Checks if the current node is an object node(has child element)*/
bool Node::isObjectNode(Document &doc)
{
    for (Node *child = this->getChild(doc); child; child = child->getNext(doc)) {
        if (child->isElementNode()) {
            return true;
        }
    }
    return false;
}

/* Checks if the current node is an element node*/
bool Node::isElementNode()
{
    return xmlNode_->type == XML_ELEMENT_NODE;
}

/* Checks if the current node ia a property node(not an object)*/
bool Node::isPropertyNode(Document &doc)
{
    return ! this->isObjectNode(doc);
}

/* Checks if the current node has any property nodes*/
bool Node::hasPropertyNode(Document &doc)
{
    for (Node *child = this->getChild(doc); child; child = child->getNext(doc)) {
        if (child->isPropertyNode(doc) && child->isElementNode()) {
            return true;
        }
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
    return xmlCharToString(xmlNodeGetContent(xmlNode_));
}

/* Returns the name of the current node */
std::string Node::getName()
{
    return xmlCharToString(xmlNode_->name);
}

/* Fills a vector with the property names of the current node */
void Node::propertyNames(std::vector<std::string> &names, Document &doc)
{
    names.clear();
    for (Node *child = this->getChild(doc); child; child = child->getNext(doc)) {
        if (child->isPropertyNode(doc) && child->isElementNode()) {
            if (child->getName() == "uuid")
                continue;
            names.push_back(child->getName());
        }
    }
}

/* Fills a vector with the property valuse of the current node*/
void Node::propertyValues(std::vector<std::string> &values, Document &doc)
{
    values.clear();
    for (Node *child = this->getChild(doc); child; child = child->getNext(doc)) {
        if (child->isPropertyNode(doc) && child->isElementNode()) {
            if (child->getName() == "uuid")
                continue;
            values.push_back(child->getContent());
        }
    }
}

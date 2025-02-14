#include "../include/parseXml.h"
/**
 *
 * Implementation for "XmlParser" Class
 *
 */

/*Process and store xml data */
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

/*Handle exception*/
void XmlParser::handleException(Client *client, const std::exception &e)
{
    std::string message = "Error : " + std::string(e.what());
    client->setResult(message);
    client->getCV().notify_one();
}

/*Process all nodes backtracking , create and insert into database*/
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

/* Insert into database */
void XmlParser::insertIntoDatabase(DatabaseManager *database, const std::string &uuid,
                                   std::vector<std::string> &names,
                                   std::vector<std::string> &values, Node *node)
{
    database->insertIntoTable(uuid, names, values, node->getName());
}

/* Create table into database */
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

/*Initialize variables*/
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

/*Determine the input type*/
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

/* Finds uuid and values mainTable */
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
/* Creates node and push it in the vector*/
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

/* Returns child of the node */
Node *Node::getChild(Document &doc)
{
    if (xmlNode_->children) {
        child = doc.createNode(xmlNode_->children);
        return child;
    }
    return nullptr;
}

/* Returns next node */
Node *Node::getNext(Document &doc)
{
    if (xmlNode_->next) {
        next = doc.createNode(xmlNode_->next);
        return next;
    }
    return nullptr;
}

/* Returns parent node */
Node *Node::getParent(Document &doc)
{
    if (xmlNode_->parent) {
        parent = doc.createNode(xmlNode_->parent);
        return parent;
    }
    return nullptr;
}

/* Checks whether the node is an object node or not */
bool Node::isObjectNode(Document &doc)
{
    for (Node *child = this->getChild(doc); child; child = child->getNext(doc)) {
        if (child->isElementNode()) {
            return true;
        }
    }
    return false;
}

/* Checks whether the node is an element node or not */
bool Node::isElementNode()
{
    return xmlNode_->type == XML_ELEMENT_NODE;
}

/* Checks whether the node is a property node or not */
bool Node::isPropertyNode(Document &doc)
{
    return ! this->isObjectNode(doc);
}

/* Checks whether the node has a property node or not */
bool Node::hasPropertyNode(Document &doc)
{
    for (Node *child = this->getChild(doc); child; child = child->getNext(doc)) {
        if (child->isPropertyNode(doc) && child->isElementNode()) {
            return true;
        }
    }
    return false;
}

/* Converts xmlChar to string */
std::string Node::xmlCharToString(const xmlChar *xml)
{
    return std::string(reinterpret_cast<const char *>(xml));
}

/* Returns the content of the node */
std::string Node::getContent()
{
    return xmlCharToString(xmlNodeGetContent(xmlNode_));
}

/* Returns the name of the node */
std::string Node::getName()
{
    return xmlCharToString(xmlNode_->name);
}

/* Fills a vector with the names of properties for the current node */
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

/* Fills a vector with the values of properties for the current node */
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

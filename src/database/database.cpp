#include "database.hpp"

namespace SQLite
{
/**
 *
 * Implementation for "Database" class
 *
 */

DatabaseManager::DatabaseManager(const DatabaseConfiguration &databaseConfiguration) :
    fileName {databaseConfiguration.getFilePath()}
{
    openDatabase();
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

/* Opens a database connection usint the specified file name */
void DatabaseManager::openDatabase()
{
    if (sqlite3_open(fileName.c_str(), &database) != SQLITE_OK) {
        std::string message = std::string("Can't open database \n") + sqlite3_errmsg(database);

        throw DatabaseException(message);
    }
}

/* Closes the database */
void DatabaseManager::closeDatabase()
{
    if (database) {
        sqlite3_close(database);
    }
}

/* Checks if a table with the given name exists	in the database.*/
bool DatabaseManager::isExistTable(const std::string &name)
{
    std::lock_guard<std::mutex> lock(dbMutex);
    std::string query = queryIsExistTable(name);

    bool exist = false;

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(database, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)

    {
        std::string message = std::string("Error preparing exist table query \n") +
                              sqlite3_errmsg(database);
        throw DatabaseException(message);
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exist = true;
    }

    sqlite3_finalize(stmt);
    return exist;
}

/* Creates a SQL query string to check if a table exists.*/
std::string DatabaseManager::queryIsExistTable(const std::string &name)
{
    std::string query = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + name +
                        "';";
    return query;
}

/* Creates a new table in the database with the specified properties */
void DatabaseManager::createTable(const std::string &name,
                                  const std::vector<std::string> &properties, bool isMainTable,
                                  const std::string &mainTable)
{
    std::lock_guard<std::mutex> lock(dbMutex);
    std::string query = queryCreate(name, properties, isMainTable, mainTable);

    char *errMsg = nullptr;

    if (sqlite3_exec(database, query.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        if (errMsg)
            sqlite3_free(errMsg);
        std::string message = std::string("Error creating table \n") + sqlite3_errmsg(database);
        throw DatabaseException(message);
    }
}

/* Creates a SQL query string to create a new table.*/
std::string DatabaseManager::queryCreate(const std::string &name,
                                         const std::vector<std::string> &properties,
                                         bool isMainTable, const std::string &mainTable)
{
    std::string query = "CREATE TABLE IF NOT EXISTS " + name + " (";

    std::string propertyQuery;

    if (isMainTable) {
        propertyQuery = "uuid TEXT PRIMARY KEY NOT NULL ";
        query += propertyQuery;

        for (const std::string property : properties) {
            propertyQuery = " , " + property + " TEXT NOT NULL  ";
            query += propertyQuery;
        }
        query += ");";
    } else {
        for (const std::string property : properties) {
            propertyQuery = property + " TEXT NOT NULL , ";
            query += propertyQuery;
        }
        query += "uuid TEXT ,"
                 "FOREIGN KEY (uuid) REFERENCES " +
                 mainTable + " (uuid));";
    }
    return query;
}

/* Inserts data into the specified table in the database.*/
void DatabaseManager::insertIntoTable(const std::string &uuid,
                                      const std::vector<std::string> &names,
                                      const std::vector<std::string> &values,
                                      const std::string &tableName)
{
    std::lock_guard<std::mutex> lock(dbMutex);

    std::string query = queryInsert(names, tableName);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(database, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::string message = std::string("Error in insert in to table \n") +
                              sqlite3_errmsg(database);

        throw DatabaseException(message);
    }

    sqlite3_bind_text(stmt, 1, uuid.c_str(), -1, SQLITE_STATIC);

    for (int i = 0; i < values.size(); i++) {
        sqlite3_bind_text(stmt, i + 2, values[i].c_str(), -1, SQLITE_STATIC);
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string message = std::string("Error executing insert \n") + sqlite3_errmsg(database);

        throw DatabaseException(message);
    }
    sqlite3_finalize(stmt);
}

/* Creates a SQL query string to insert values into the specified table.*/
std::string DatabaseManager::queryInsert(const std::vector<std::string> &names,
                                         const std::string &tableName)
{
    std::string query = "INSERT INTO " + tableName + " (uuid";
    for (const std::string name : names) {
        query += ", " + name;
    }
    query += ") VALUES (? ";
    for (int i = 0; i < names.size(); i++) {
        query += ",?";
    }
    query += ");";

    return query;
}

/* Retrieves all table names from the database.*/
std::vector<std::string> DatabaseManager::getAllTableNames()
{
    std::vector<std::string> tableNames;

    std::string query = queryAllTableNames();

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(database, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::string message = "Error fetching table names: " +
                              std::string(sqlite3_errmsg(database));
        throw DatabaseException(message);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *tableName = (const char *) sqlite3_column_text(stmt, 0);

        if (tableName)
            tableNames.push_back(tableName);
    }
    sqlite3_finalize(stmt);

    return tableNames;
}

/* Creates a SQL query string to get all table names.*/
std::string DatabaseManager::queryAllTableNames()
{
    std::string query = "SELECT name FROM sqlite_master WHERE type='table';";
    return query;
}

/* Fetches data from the specified table and returns it as an XML string.*/
std::string DatabaseManager::fetchTableDataAsXML(const std::string &tableName)
{
    std::lock_guard<std::mutex> lock(dbMutex);
    std::ostringstream xmlStream;

    xmlStream << "<" << tableName << ">\n";

    std::string query = querySelect(tableName);

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(database, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::string message = "Error preparing select query: " +
                              std::string(sqlite3_errmsg(database));
        throw DatabaseException(message);
    }

    int columnCount = sqlite3_column_count(stmt);

    if (columnCount < 1) {
        sqlite3_finalize(stmt);
        return "<" + tableName + " />\n";
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        for (int i = 0; i < columnCount; ++i) {
            const char *columnName = sqlite3_column_name(stmt, i);
            const char *columnValue = (const char *) sqlite3_column_text(stmt, i);

            xmlStream << "    <" << columnName << ">" << (columnValue ? columnValue : "NULL")
                      << "</" << columnName << ">\n";
        }
    }

    sqlite3_finalize(stmt);
    xmlStream << "</" << tableName << ">\n";

    return xmlStream.str();
}

/* Creates a SQL query string to SELECT table*/
std::string DatabaseManager::querySelect(const std::string &tableName)
{
    std::string query = "SELECT * FROM " + tableName + ";";
    return query;
}

/* Fetches data from all tables in the database and returns it as an XML string.*/
std::string DatabaseManager::fetchAllTablesAsXML()
{
    std::ostringstream xmlStream;

    xmlStream << "<database>\n";

    std::vector<std::string> tableNames = getAllTableNames();
    for (const std::string &tableName : tableNames) {
        xmlStream << fetchTableDataAsXML(tableName);
    }

    xmlStream << "</database>\n";
    return xmlStream.str();
}

sqlite3 *DatabaseManager::getDatabase() const
{
    return database;
}

} /* namespace SQLite */

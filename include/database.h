/**
 * \file : database.h
 *
 *
 * \author : MohammadDerhami
 *
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "../include/config.h"

#include <cstring>
#include <iostream>
#include <mutex>
#include <sqlite3.h>
#include <sstream>
#include <vector>

/**
 * DatabaseManager Class
 */
class DatabaseManager
{
public:
    /*Constructor*/
    DatabaseManager(const DatabaseConfiguration &databaseConfiguration)

    {
        fileName = databaseConfiguration.getFilePath();
        openDatabase();
    }

    /*Destructor*/
    ~DatabaseManager()
    {
        closeDatabase();
    }

    /*Getter for database*/
    sqlite3 *getDatabase() const
    {
        return database;
    }

    /*
     * @brief Checks whether a table with this name exists or not.
     *
     * @param The name that should be checked in the table names.
     *
     * @return true if exist , false if not exist
     *
     * @note Checks how many tables named name exist, and returns true if
     * the number is greater than 0.
     *
     * @warning This method throws DatabaseException  if query cannot be
     * prepared.
     */
    bool isExistTable(const std::string &name);

    /*
     * @brief Create table in database
     *
     * @param name of table , vector of properties , whether this table is
     * main table , name of main table
     *
     * @warning This method throws DatabaseException if query cannot be
     * execute.
     */
    void createTable(const std::string &name, const std::vector<std::string> &properties,
                     bool isMainTable, const std::string &mainTable);

    /*
     * @brief Inserts properties value in the table.
     *
     * @param uuid of xml data , names vector of name of properties , values
     * vector of value of properties , name of table.
     *
     * @warning This method throws DatabaseException if query cannot be
     * prepare or step.
     */

    void insertIntoTable(const std::string &uuid, const std::vector<std::string> &names,
                         const std::vector<std::string> &values, const std::string &tableName);

    /*
     * @brief Fetch all data in database
     * @return string of xml data
     */
    std::string fetchAllTablesAsXML();

private:
    /*Mutex*/
    std::mutex dbMutex;

    /* Name of database file*/
    std::string fileName;

    /* Object from database */
    sqlite3 *database;

    /*
     * @brief open database.
     * @warning This method throws DatabaseException if database doesnt
     * open.
     */
    void openDatabase();

    /*
     * @brief cloese database.
     */
    void closeDatabase();

    /*
     * @brief Create query for isExistTable method.
     * @param  name of table
     * @return query
     */
    std::string queryIsExistTable(const std::string &name);

    /*
     * @brief Create query for createTable method
     * @param name of table , properties of table ,
     * whether this table is main table , name of main table.
     * @return query
     */
    std::string queryCreateTable(const std::string &name,
                                 const std::vector<std::string> &properties, bool isMainTable,
                                 const std::string &mainTable);

    /*
     * @brief Create query for insert into table.
     * @param vector of name of properties , vector of value of properties ,
     * name of table.
     * @return query
     */
    std::string queryInsertIntoTable(const std::vector<std::string> &names,
                                     const std::string &tableName);
    /*
     * @brief store name of tables in the vector.
     * @return vector of names.
     */
    std::vector<std::string> getAllTableNames();

    /*
     * @brief Fetch data of table as xml
     * @param name of table
     * @return xml data
     * @warning This method throws DatabaseException if :
     * -query does not prepare
     * -table is empty
     */
    std::string fetchTableDataAsXML(const std::string &tableName);
};
/**
 * DatabaseException Class
 *
 * Handles database exceptions
 */
class DatabaseException : public std::runtime_error
{
public:
    DatabaseException(const std::string &message) : std::runtime_error(message)
    {
    }
};

#endif

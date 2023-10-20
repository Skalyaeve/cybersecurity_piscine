#ifndef SQLI_MICROSOFTSQL
#define SQLI_MICROSOFTSQL

#include "blind_based.hpp"
#include "error_based.hpp"
#include "union_based.hpp"

struct sqli_microsoftsql
{
        inline static const str_vector excluded = {
            "master",
            "msdb",
            "model",
            "tempdb",
        };

        static std::string payload(
            sptr_vector& config,
            uint8 method_type,
            uint8 value_type)
        {
                switch (method_type)
                {
                case UNION:
                        return microsoftsql_union_based::payload(config, value_type);
                case ERROR:
                        return microsoftsql_error_based::payload(config, value_type);
                case BLIND:
                        return microsoftsql_blind_based::payload(config, value_type);
                default:
                        return std::string();
                }
        };

        static str_vector parser(
            const std::string& response,
            sptr_vector& config,
            const uint8& method_type)
        {
                return sqli_mysql::parser(
                    response,
                    config,
                    method_type);
        }
};
#endif

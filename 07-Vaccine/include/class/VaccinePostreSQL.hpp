#ifndef VACCINE_POSTGRESQL
#define VACCINE_POSTGRESQL

#include "VaccineBase.hpp"
#include <algorithm>

class VaccinePostreSQL : virtual public VaccineBase
{
public:
        VaccinePostreSQL(
            const std::string& url,
            const std::string& request_type,
            const std::vector< std::string >& headers,
            const std::string& archive_file);

        VaccinePostreSQL(const VaccinePostreSQL& other);
        virtual ~VaccinePostreSQL();
        VaccinePostreSQL& operator=(const VaccinePostreSQL& other);

        void stacked_queries();
        void union_based();
        void error_based();
        void blind_based();

protected:
        VaccinePostreSQL();

        void _fetch_databases(
            const std::vector< std::string >& base_parameters,
            std::vector< std::string >& payloads,
            std::string& offset,
            const std::string& url);

        void _fetch_tables(
            const std::vector< std::string >& base_parameters,
            std::vector< std::string >& payloads,
            std::string& offset,
            const std::string& url,
            const std::string& schema_name);

        void _fetch_columns(
            const std::vector< std::string >& base_parameters,
            std::vector< std::string >& payloads,
            std::string& offset,
            const std::string& url,
            const std::string& schema_name,
            const int& tab_index);

        void _fetch_values(
            const std::vector< std::string >& base_parameters,
            std::vector< std::string >& payloads,
            std::string& offset,
            const std::string& url,
            const std::string& schema_name,
            const int& tab_index);
};
#endif

#ifndef VACCINE_MYSQL
#define VACCINE_MYSQL

#include "VaccineBase.hpp"
#include <algorithm>

class VaccineMySQL : virtual public VaccineBase
{
public:
        VaccineMySQL(
            const std::string& url,
            const std::string& request_type,
            const std::vector< std::string >& headers,
            const std::string& archive_file);

        VaccineMySQL(const VaccineMySQL& other);
        virtual ~VaccineMySQL();
        VaccineMySQL& operator=(const VaccineMySQL& other);

        void stacked_queries();
        void union_based();
        void error_based();
        void blind_based();

protected:
        VaccineMySQL();

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
            const std::string& db_name);

        void _fetch_columns(
            const std::vector< std::string >& base_parameters,
            std::vector< std::string >& payloads,
            std::string& offset,
            const std::string& url,
            const std::string& db_name,
            const int& tab_index);

        void _fetch_values(
            const std::vector< std::string >& base_parameters,
            std::vector< std::string >& payloads,
            std::string& offset,
            const std::string& url,
            const std::string& db_name,
            const int& tab_index);
};
#endif

#ifndef VACCINE_SQLITE
#define VACCINE_SQLITE

#include "VaccineBase.hpp"
#include <algorithm>

class VaccineSQLite : virtual public VaccineBase
{
public:
        VaccineSQLite(
            const std::string& url,
            const std::string& request_type,
            const std::vector< std::string >& headers,
            const std::string& archive_file);

        VaccineSQLite(const VaccineSQLite& other);
        virtual ~VaccineSQLite();
        VaccineSQLite& operator=(const VaccineSQLite& other);

        void stacked_queries();
        void union_based();
        void error_based();
        void blind_based();

protected:
        VaccineSQLite();

        void _fetch_tables(
            const std::vector< std::string >& base_parameters,
            std::vector< std::string >& payloads,
            std::string& offset,
            const std::string& url);

        void _fetch_columns(
            const std::vector< std::string >& base_parameters,
            std::vector< std::string >& payloads,
            std::string& offset,
            const std::string& url,
            const int& tab_index);

        void _fetch_values(
            const std::vector< std::string >& base_parameters,
            std::vector< std::string >& payloads,
            std::string& offset,
            const std::string& url,
            const int& tab_index);

        std::vector< std::string > _parse_response(
            const std::string& response,
            bool is_col = false) const;

        std::vector< std::string > _parse_col_response(
            const Json::Value& entries,
            const std::string& member) const;
};
#endif

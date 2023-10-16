#ifndef VACCINE_SQLI
#define VACCINE_SQLI

#include "Vaccine.hpp"
#include "sqli_microsoftsql.hpp"
#include "sqli_mysql.hpp"
#include "sqli_postgresql.hpp"
#include "sqli_sqlite.hpp"

class VaccineSQLi : public Vaccine
{
public:
        VaccineSQLi(
            const uint8& db_type,
            const std::string& url,
            const std::string& request_type,
            const str_vector& headers,
            const std::string& archive_file);

        VaccineSQLi(const VaccineSQLi& other);
        virtual ~VaccineSQLi();
        VaccineSQLi& operator=(const VaccineSQLi& other);

        void stacked_queries();
        void union_based();
        void error_based();
        void blind_based();

protected:
        std::string _full_url;
        str_vector _parameters;
        str_vector _payloads;
        FtGetPayload _ft_payload;
        FtParseResponse _ft_response;
        str_vector _excluded;
        uint8 _method_type;

        VaccineSQLi();
        void _worker_init();

        void _launch(t_config config);
        std::string _sqli_method_tostr();

        void _get_payload(uint8 value_type);
        void _get_parser(uint8 value_type);

        void _fetch(
            uint8 value_type,
            t_config& config,
            const std::string& db_name,
            const int& tab_index);
        void _fetch_databases(
            sptr_vector config);
        void _fetch_tables(
            sptr_vector config,
            const std::string& db_name);
        void _fetch_columns(
            sptr_vector config,
            const std::string& db_name,
            const int& tab_index);
        void _fetch_values(
            sptr_vector config,
            const std::string& db_name,
            const int& tab_index);
};

#endif
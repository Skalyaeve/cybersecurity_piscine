#ifndef VACCINE_SQLI
#define VACCINE_SQLI

#define TIMEOUT 32

#include <functional>
#include <json/json.h>
#include <libxml/HTMLparser.h>

#include "../Vaccine.hpp"

#include "microsoftsql/microsoftsql.hpp"
#include "mysql/mysql.hpp"
#include "postgresql/postgresql.hpp"
#include "sqlite/sqlite.hpp"

typedef std::vector< sptr_vector > t_config;

using FtPayload = std::function< std::string(
    sptr_vector&,
    const uint8&,
    const uint8&) >;

using FtParser = std::function< str_vector(
    const std::string&,
    sptr_vector&,
    const uint8&,
    const uint8&) >;

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

        bool fetch_data();
        void stacked_queries();
        void union_based();
        void error_based();
        void blind_based();

private:
        htmlDocPtr* _html_doc;
        std::vector< xmlNode* > _gates;

        uint8 _method_type;
        std::string _full_url;
        str_vector _parameters;
        str_vector _payloads;
        str_vector _excluded;
        FtPayload _ft_payload;
        FtParser _ft_parser;

        VaccineSQLi();
        void _worker_init();

        // ================================ INIT
        void _parse_html(xmlNode* node);
        void _launch(t_config base_config);
        bool _init_launch();

        bool _data_check() const;
        std::string _sqli_method_tostr() const;

        bool _init_attempt(
            const xmlNode* node,
            std::string& url,
            str_vector& parameters,
            t_config& config);
        void _parse_form(
            const xmlNode* node,
            str_vector& parameters) const;
        void _parse_link(
            const xmlNode* node,
            str_vector& parameters) const;

        // ================================ PREPARE
        void _fetch(
            sptr_vector& config,
            const uint8& value_type,
            const std::string& db_name,
            const int& tab_index);

        void _fetch_databases(
            sptr_vector& config,
            const uint8& value_type);
        void _fetch_tables(
            sptr_vector& config,
            const uint8& value_type,
            std::string& db_name);
        void _fetch_columns(
            sptr_vector& config,
            const uint8& value_type,
            const int& tab_index);
        void _fetch_values(
            sptr_vector& config,
            const uint8& value_type,
            const int& tab_index);

        str_vector _fill_parameters(
            const str_vector& old_parameters,
            const std::string& payload) const;

        // ================================ PROCESS
        std::string _process(
            const std::string& url,
            const str_vector& parameters) const;
        void _process_get(
            const std::string& url,
            const str_vector& parameters,
            CURL** curl) const;
        void _process_post(
            const str_vector& parameters,
            CURL** curl) const;

        // ====================s============ MANAGE
        bool _manage(
            const uint8& value_type,
            const str_vector& response,
            sptr_vector& config,
            const int* index,
            const std::string* name);
        bool _manage_blind_based(
            const uint8& value_type,
            const str_vector& response,
            sptr_vector& config,
            const int* tab_index,
            const std::string* tab_name);
};
#endif
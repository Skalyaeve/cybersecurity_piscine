#ifndef VACCINE_BASE
#define VACCINE_BASE

#define ALL 0
#define MYSQL 1
#define POSTGRESQL 2
#define SQLITE 3
#define MICROSOFTSQL 4

#define TIMEOUT 32

#include <ctime>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <libxml/HTMLparser.h>
#include <unordered_map>
#include <vector>

class VaccineBase
{
public:
        VaccineBase(
            const short& db_type,
            const std::string& url,
            const std::string& request_type,
            const std::vector< std::string >& headers,
            const std::string& archive_file);

        VaccineBase(const VaccineBase& other);
        virtual ~VaccineBase();
        VaccineBase& operator=(const VaccineBase& other);

        // =========================================== GETTERS
        const short& get_db_type() const;
        const std::string get_str_db_type() const;
        const std::string& get_url() const;
        const std::string& get_request_type() const;
        const std::vector< std::string >& get_headers() const;
        const std::string& get_archive_file() const;

        const std::string& get_html() const;
        const htmlDocPtr* get_html_doc() const;
        const std::vector< xmlNode* >& get_gates() const;

        const std::unordered_map< std::string, std::vector< int > >& get_databases() const;
        const std::vector< std::unordered_map< std::string, int > >& get_tables() const;
        const std::vector< std::unordered_map< std::string, int > >& get_columns() const;
        const std::vector< std::vector< std::string > >& get_values() const;

        // =========================================== SETTERS
        void set_db_type(const short& db_type);
        void set_url(const std::string& url);
        void set_request_type(const std::string& request_type);
        void set_headers(const std::vector< std::string >& headers);
        void set_archive_file(const std::string& archive_file);

        // =========================================== METHODS
        bool fetch_data();

protected:
        inline static const std::vector< std::string > _excluded = {
            "information_schema",
            "mysql",
            "performance_schema",
            "sys",
            "pg_catalog",
            "pg_toast",
            "sqlite_sequence",
            "master",
            "msdb",
            "model",
            "tempdb",
        };
        std::string _url;
        std::string _request_type;
        std::vector< std::string > _headers;
        std::string _archive_file;

        std::string _html;
        htmlDocPtr* _html_doc;
        std::vector< xmlNode* > _gates;

        std::unordered_map< std::string, std::vector< int > > _databases;
        std::vector< std::unordered_map< std::string, int > > _tables;
        std::vector< std::unordered_map< std::string, int > > _columns;
        std::vector< std::vector< std::string > > _values;

        short _db_type;
        VaccineBase();

        // =========================================== INIT
        bool _fetch_html();
        void _parse_html(xmlNode* node);
        bool _data_check() const;

        bool _init_attempt(
            const xmlNode* node,
            std::string& url,
            std::vector< std::string >& parameters) const;
        void _parse_form(
            const xmlNode* node,
            std::vector< std::string >& parameters) const;
        void _parse_link(
            const xmlNode* node,
            std::vector< std::string >& parameters) const;

        // =========================================== PREPARE
        std::vector< std::string > _fill_parameters(
            const std::vector< std::string >& old_parameters,
            const std::string& payload) const;

        // =========================================== PROCESS
        std::string _process(
            const std::string& url,
            const std::vector< std::string >& parameters) const;
        void _process_get(
            const std::string& url,
            const std::vector< std::string >& parameters,
            CURL** curl) const;
        void _process_post(
            const std::vector< std::string >& parameters,
            CURL** curl) const;
        std::vector< std::string > _parse_response(
            const std::string& response) const;

        // =========================================== SAVE
        void _save_results(
            const std::string& url,
            const std::vector< std::string >& parameters,
            const std::vector< std::string > payloads) const;
        void _save_table(
            std::ofstream& file,
            const int& tab_index,
            const std::string& table_name) const;

        // =========================================== STATICS
        static size_t _curl_callback(
            char* ptr,
            size_t size,
            size_t nmemb,
            void* buffer);
};
#endif

#ifndef VACCINE
#define VACCINE

#define ALL 0
#define MYSQL 1
#define POSTGRESQL 2
#define SQLITE 3
#define MICROSOFTSQL 4

#define STACKED 0
#define UNION 1
#define ERROR 2
#define BLIND 3

#define DATABASES 0
#define TABLES 1
#define COLUMNS 2
#define VALUES 3

#define TIMEOUT 32

#include <ctime>
#include <curl/curl.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <json/json.h>
#include <libxml/HTMLparser.h>
#include <unordered_map>
#include <vector>

typedef unsigned char uint8;
typedef struct curl_slist curl_slist;
typedef std::vector< std::string > str_vector;
typedef std::vector< std::string* > sptr_vector;

typedef std::unordered_map< std::string, std::vector< int > > t_databases;
typedef std::vector< std::unordered_map< std::string, int > > t_tables;
typedef std::vector< std::unordered_map< std::string, int > > t_columns;
typedef std::vector< str_vector > t_values;
typedef std::vector< sptr_vector > t_config;

using FtGetPayload = std::function< std::string(const sptr_vector&) >;
using FtParseResponse = std::function< str_vector(const std::string&) >;

class Vaccine
{
public:
        Vaccine(
            const uint8& db_type,
            const std::string& url,
            const std::string& request_type,
            const str_vector& headers,
            const std::string& archive_file);

        Vaccine(const Vaccine& other);
        virtual ~Vaccine();
        Vaccine& operator=(const Vaccine& other);

        // ================================ GETTERS
        const short& get_db_type() const;
        const std::string db_type_tostr() const;
        const std::string& get_url() const;
        const std::string& get_request_type() const;
        const str_vector& get_headers() const;
        const std::string& get_archive_file() const;

        const std::string& get_html() const;
        const htmlDocPtr* get_html_doc() const;
        const std::vector< xmlNode* >& get_gates() const;

        const t_databases& get_databases() const;
        const t_tables& get_tables() const;
        const t_columns& get_columns() const;
        const t_values& get_values() const;

        // ================================ SETTERS
        void set_db_type(const short& db_type);
        void set_url(const std::string& url);
        void set_request_type(const std::string& request_type);
        void set_headers(const str_vector& headers);
        void set_archive_file(const std::string& archive_file);

        // ================================ METHODS
        bool fetch_data();

protected:
        uint8 _db_type;
        std::string _url;
        std::string _request_type;
        str_vector _headers;
        std::string _archive_file;

        std::string _html;
        htmlDocPtr* _html_doc;
        std::vector< xmlNode* > _gates;

        t_databases _databases;
        t_tables _tables;
        t_columns _columns;
        t_values _values;

        Vaccine();

        // ================================ INIT
        bool _fetch_html();
        void _parse_html(xmlNode* node);
        bool _data_check() const;

        bool _init_attempt(
            const xmlNode* node,
            std::string& url,
            str_vector& parameters) const;
        void _parse_form(
            const xmlNode* node,
            str_vector& parameters) const;
        void _parse_link(
            const xmlNode* node,
            str_vector& parameters) const;

        // ================================ PREPARE
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

        // ================================ SAVE
        void _save_results(
            const std::string& url,
            const str_vector& parameters,
            const str_vector payloads) const;
        void _save_table(
            std::ofstream& file,
            const int& tab_index,
            const std::string& table_name) const;

        // ================================ STATICS
        static size_t _curl_callback(
            char* ptr,
            size_t size,
            size_t nmemb,
            void* buffer);
};
#endif

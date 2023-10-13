#ifndef VACCINE
#define VACCINE

#include <curl/curl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <json/json.h>
#include <libxml/HTMLparser.h>
#include <map>
#include <sstream>
#include <vector>

class Vaccine
{
public:
        Vaccine(
            const std::string& url,
            const std::string& archive_file,
            const std::string& request_type,
            const std::vector< std::string >& headers,
            const short& _db_type);
        Vaccine(const Vaccine& other);
        virtual ~Vaccine();

        Vaccine& operator=(const Vaccine& other);

        const std::string& get_url() const;
        const std::string& get_archive_file() const;
        const std::string& get_request_type() const;
        const std::vector< std::string >& get_headers() const;
        const short& get_db_type() const;

        const std::string& get_html() const;
        const htmlDocPtr* get_html_doc() const;
        const std::vector< xmlNode* >& get_forms() const;

        const std::map< std::string, std::vector< int > >& get_databases() const;
        const std::vector< std::map< std::string, int > >& get_tables() const;
        const std::vector< std::vector< std::string > >& get_columns() const;

        void set_url(const std::string& url);
        void set_archive_file(const std::string& archive_file);
        void set_request_type(const std::string& request_type);
        void set_headers(const std::vector< std::string >& headers);
        void set_db_type(const short& db_type);

        bool fetch_data();

        void union_based();

protected:
        std::string _url;
        std::string _archive_file;
        std::string _request_type;
        std::vector< std::string > _headers;
        short _db_type;

        std::string _html;
        htmlDocPtr* _html_doc;
        std::vector< xmlNode* > _forms;

        std::map< std::string, std::vector< int > > _databases;
        std::vector< std::map< std::string, int > > _tables;
        std::vector< std::vector< std::string > > _columns;

        Vaccine();

        bool _fetch_html();
        void _parse_html(xmlNode* node);
        bool _data_check() const;

        void _get_form_uri(
            const xmlNode* node,
            std::string& uri) const;
        void _get_form_entries(
            const xmlNode* node,
            std::vector< std::string >& entries) const;
        std::vector< std::string > _fill_entries(
            const std::vector< std::string >& base_entries,
            const std::string& payload) const;

        std::string _process(
            const std::string& full_url,
            const std::vector< std::string >& entries) const;
        void _process_get(
            const std::string& full_url,
            const std::vector< std::string >& entries,
            CURL** curl) const;
        void _process_post(
            const std::vector< std::string >& entries,
            CURL** curl) const;

        bool _fill_databases(
            const std::string& response,
            const std::vector< std::string >& excluded);
        void _fill_tables(
            const std::string& response,
            const std::string& db);
        void _fill_columns(
            const std::string& response,
            const int& table_index,
            const std::string& table);

        void _save_results(
            const std::string& full_url,
            const xmlNode* gate,
            std::vector< std::string > payloads) const;

        static size_t _curl_callback(
            char* ptr,
            size_t size,
            size_t nmemb,
            void* buffer);
};
#endif

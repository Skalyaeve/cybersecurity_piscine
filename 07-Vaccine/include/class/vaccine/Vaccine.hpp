#ifndef VACCINE
#define VACCINE

#define ALL 0
#define MYSQL 1
#define POSTGRESQL 2
#define SQLITE 3
#define MICROSOFTSQL 4

#define DATABASES 0
#define TABLES 1
#define COLUMNS 2
#define VALUES 3

#include <ctime>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

typedef unsigned char uint8;
typedef std::vector< std::string > str_vector;
typedef std::vector< std::string* > sptr_vector;
typedef std::vector< void* > ptr_vector;

typedef std::unordered_map< std::string, std::vector< int > > t_databases;
typedef std::vector< std::unordered_map< std::string, int > > t_tables;
typedef std::vector< std::unordered_map< std::string, int > > t_columns;
typedef std::vector< str_vector > t_values;

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

        const std::string db_type_tostr() const;
        const std::string& get_url() const;
        const std::string& get_request_type() const;
        const str_vector& get_headers() const;
        const std::string& get_archive_file() const;

        const std::string& get_html() const;
        const t_databases& get_databases() const;
        const t_tables& get_tables() const;
        const t_columns& get_columns() const;
        const t_values& get_values() const;

        void set_db_type(const short& db_type);
        void set_url(const std::string& url);
        void set_request_type(const std::string& request_type);
        void set_headers(const str_vector& headers);
        void set_archive_file(const std::string& archive_file);

protected:
        uint8 _db_type;
        std::string _url;
        std::string _request_type;
        str_vector _headers;
        std::string _archive_file;

        std::string _html;
        t_databases _databases;
        t_tables _tables;
        t_columns _columns;
        t_values _values;

        Vaccine();

        bool _fetch_html();

        void _save_results(
            const std::string& url,
            const str_vector& parameters,
            const str_vector payloads,
            const std::string& via) const;
        void _save_table(
            std::ofstream& file,
            const int& tab_index,
            const std::string& table_name) const;

        static size_t _curl_callback(
            char* ptr,
            size_t size,
            size_t nmemb,
            void* buffer);
};
#endif

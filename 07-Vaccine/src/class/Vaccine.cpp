#include "../../include/class/vaccine/Vaccine.hpp"

Vaccine::Vaccine()
{
        this->_db_type = MYSQL;
        this->_url = std::string();
        this->_request_type = std::string();
        this->_headers = str_vector();
        this->_archive_file = std::string();

        this->_html = std::string();
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
        this->_databases = t_databases();
        this->_tables = std::vector< std::unordered_map< std::string, int > >();
        this->_columns = std::vector< std::unordered_map< std::string, int > >();
        this->_values = std::vector< str_vector >();
}

Vaccine::Vaccine(const Vaccine& other)
{
        *this = other;
}

Vaccine::~Vaccine() {}

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
                this->_databases = other._databases;
                this->_tables = other._tables;
                this->_columns = other._columns;
                this->_values = other._values;
        }
        return *this;
}

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

// ===================================================================== //
// =========================== P R I V A T E =========================== //
// ===================================================================== //

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

void Vaccine::_save_results(
    const std::string& url,
    const str_vector& parameters,
    const str_vector payloads,
    const std::string& via) const
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
             << this->_url << " via " << via
             << " (as " << this->db_type_tostr() << ")\": {" << std::endl;
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

// ===================================================================== //
// ============================ S T A T I C ============================ //
// ===================================================================== //

size_t Vaccine::_curl_callback(
    char* ptr,
    size_t size,
    size_t nmemb,
    void* buffer)
{
        ((std::string*)buffer)->append((char*)ptr, size * nmemb);
        return size * nmemb;
}

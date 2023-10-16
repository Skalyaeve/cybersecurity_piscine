#include "../../include/class/Vaccine.hpp"

Vaccine::Vaccine()
    : VaccineBase(),
      VaccineMySQL(),
      VaccinePostreSQL(),
      VaccineSQLite(),
      VaccineMicrosoftSQL() {}

Vaccine::Vaccine(
    const std::string& url,
    const std::string& request_type,
    const std::vector< std::string >& headers,
    const std::string& archive_file)
    : VaccineBase(MYSQL, url, request_type, headers, archive_file),
      VaccineMySQL(url, request_type, headers, archive_file),
      VaccinePostreSQL(url, request_type, headers, archive_file),
      VaccineSQLite(url, request_type, headers, archive_file),
      VaccineMicrosoftSQL(url, request_type, headers, archive_file) {}

Vaccine::Vaccine(const Vaccine& other)
    : VaccineBase(other),
      VaccineMySQL(other),
      VaccinePostreSQL(other),
      VaccineSQLite(other),
      VaccineMicrosoftSQL(other) {}

Vaccine::~Vaccine() {}

Vaccine& Vaccine::operator=(const Vaccine& other)
{
        if (this != &other)
        {
                VaccineBase::operator=(other);
                VaccineMySQL::operator=(other);
                VaccinePostreSQL::operator=(other);
                VaccineSQLite::operator=(other);
                VaccineMicrosoftSQL::operator=(other);
        }
        return *this;
}

void Vaccine::stacked_queries(const short& db_type)
{
        std::cout << std::endl
                  << "Processing stacked queries..." << std::endl;
        this->_db_type = db_type;
        switch (db_type)
        {
        case MYSQL:
                this->VaccineMySQL::stacked_queries();
                break;
        case POSTGRESQL:
                this->VaccinePostreSQL::stacked_queries();
                break;
        case SQLITE:
                this->VaccineSQLite::stacked_queries();
                break;
        case MICROSOFTSQL:
                this->VaccineMicrosoftSQL::stacked_queries();
                break;
        default:
                this->_db_type = MYSQL;
                this->VaccineMySQL::stacked_queries();
                this->_db_type = POSTGRESQL;
                this->VaccinePostreSQL::stacked_queries();
                this->_db_type = SQLITE;
                this->VaccineSQLite::stacked_queries();
                this->_db_type = MICROSOFTSQL;
                this->VaccineMicrosoftSQL::stacked_queries();
                this->_db_type = db_type;
        }
        std::cout << "[ DONE ]" << std::endl;
}

void Vaccine::union_based(const short& db_type)
{
        std::cout << std::endl
                  << "Processing union based..." << std::endl;
        this->_db_type = db_type;
        switch (db_type)
        {
        case MYSQL:
                this->VaccineMySQL::union_based();
                break;
        case POSTGRESQL:
                this->VaccinePostreSQL::union_based();
                break;
        case SQLITE:
                this->VaccineSQLite::union_based();
                break;
        case MICROSOFTSQL:
                this->VaccineMicrosoftSQL::union_based();
                break;
        default:
                this->_db_type = MYSQL;
                this->VaccineMySQL::union_based();
                this->_db_type = POSTGRESQL;
                this->VaccinePostreSQL::union_based();
                this->_db_type = SQLITE;
                this->VaccineSQLite::union_based();
                this->_db_type = MICROSOFTSQL;
                this->VaccineMicrosoftSQL::union_based();
                this->_db_type = db_type;
        }
        std::cout << "[ DONE ]" << std::endl;
}

void Vaccine::error_based(const short& db_type)
{
        std::cout << std::endl
                  << "Processing error based..." << std::endl;
        this->_db_type = db_type;
        switch (db_type)
        {
        case MYSQL:
                this->VaccineMySQL::error_based();
                break;
        case POSTGRESQL:
                this->VaccinePostreSQL::error_based();
                break;
        case SQLITE:
                this->VaccineSQLite::error_based();
                break;
        case MICROSOFTSQL:
                this->VaccineMicrosoftSQL::error_based();
                break;
        default:
                this->_db_type = MYSQL;
                this->VaccineMySQL::error_based();
                this->_db_type = POSTGRESQL;
                this->VaccinePostreSQL::error_based();
                this->_db_type = SQLITE;
                this->VaccineSQLite::error_based();
                this->_db_type = MICROSOFTSQL;
                this->VaccineMicrosoftSQL::error_based();
                this->_db_type = db_type;
        }
        std::cout << "[ DONE ]" << std::endl;
}

void Vaccine::blind_based(const short& db_type)
{
        std::cout << std::endl
                  << "Processing blind based..." << std::endl;
        this->_db_type = db_type;
        switch (db_type)
        {
        case MYSQL:
                this->VaccineMySQL::blind_based();
                break;
        case POSTGRESQL:
                this->VaccinePostreSQL::blind_based();
                break;
        case SQLITE:
                this->VaccineSQLite::blind_based();
                break;
        case MICROSOFTSQL:
                this->VaccineMicrosoftSQL::blind_based();
                break;
        default:
                this->_db_type = MYSQL;
                this->VaccineMySQL::blind_based();
                this->_db_type = POSTGRESQL;
                this->VaccinePostreSQL::blind_based();
                this->_db_type = SQLITE;
                this->VaccineSQLite::blind_based();
                this->_db_type = MICROSOFTSQL;
                this->VaccineMicrosoftSQL::blind_based();
                this->_db_type = db_type;
        }
        std::cout << "[ DONE ]" << std::endl;
}
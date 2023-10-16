#ifndef VACCINE
#define VACCINE

#include "VaccineMicrosoftSQL.hpp"
#include "VaccineMySQL.hpp"
#include "VaccinePostreSQL.hpp"
#include "VaccineSQLite.hpp"

class Vaccine
    : public VaccineMySQL,
      public VaccinePostreSQL,
      public VaccineSQLite,
      public VaccineMicrosoftSQL
{
public:
        Vaccine(
            const std::string& url,
            const std::string& request_type,
            const std::vector< std::string >& headers,
            const std::string& archive_file);

        Vaccine(const Vaccine& other);
        virtual ~Vaccine();

        Vaccine& operator=(const Vaccine& other);

        void stacked_queries(const short& db_type);
        void union_based(const short& db_type);
        void error_based(const short& db_type);
        void blind_based(const short& db_type);

protected:
        Vaccine();
};

#endif
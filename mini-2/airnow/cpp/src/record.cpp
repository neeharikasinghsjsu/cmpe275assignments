#include "record.hpp"

// Default constructor
Record::Record() {}

// Parameterized constructor
Record::Record(double lat, double lon, const std::chrono::system_clock::time_point &time,
               const std::string &poll, double conc, const std::string &unit,
               double raw_conc, int aqi_val, int cat, const std::string &site,
               const std::string &agency, const std::string &aqs, const std::string &country_aqs)
    : latitude(lat), longitude(lon), recordTime(time), pollutant(poll), concentration(conc),
      pollutantUnit(unit), rawConcentration(raw_conc), aqi(aqi_val), category(cat),
      siteName(site), siteAgency(agency), aqsId(aqs), countryCodeAqsId(country_aqs) {}

// Getter methods
double Record::getLatitude() const { return latitude; }
double Record::getLongitude() const { return longitude; }
std::chrono::system_clock::time_point Record::getRecordTime() const { return recordTime; }
std::string Record::getPollutant() const { return pollutant; }
double Record::getConcentration() const { return concentration; }
std::string Record::getPollutantUnit() const { return pollutantUnit; }
double Record::getRawConcentration() const { return rawConcentration; }
int Record::getAQI() const { return aqi; }
int Record::getCategory() const { return category; }
std::string Record::getSiteName() const { return siteName; }
std::string Record::getSiteAgency() const { return siteAgency; }
std::string Record::getAQSId() const { return aqsId; }
std::string Record::getCountryCodeAQSId() const { return countryCodeAqsId; }

// Setter methods
void Record::setLatitude(double lat)
{
    latitude = lat;
}
void Record::setLongitude(double lon)
{
    longitude = lon;
}
void Record::setRecordTime(const std::chrono::system_clock::time_point &time)
{
    recordTime = time;
}
void Record::setPollutant(const std::string &poll)
{
    pollutant = poll;
}
void Record::setConcentration(double conc)
{
    concentration = conc;
}
void Record::setPollutantUnit(const std::string &unit)
{
    pollutantUnit = unit;
};
void Record::setRawConcentration(double raw_conc)
{
    rawConcentration = raw_conc;
}
void Record::setAQI(int aqi_val)
{
    aqi = aqi_val;
}
void Record::setCategory(int cat)
{
    category = cat;
}
void Record::setSiteName(const std::string &site)
{
    siteName = site;
}
void Record::setSiteAgency(const std::string &agency)
{
    siteAgency = agency;
}
void Record::setAQSId(const std::string &aqs)
{
    aqsId = aqs;
}
void Record::setCountryCodeAQSId(const std::string &country_aqs)
{
    countryCodeAqsId = country_aqs;
}

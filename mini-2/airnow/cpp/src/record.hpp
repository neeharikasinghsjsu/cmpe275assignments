#ifndef RECORD_HPP
#define RECORD_HPP

#include <chrono>
#include <string>

class Record {
private:
    double latitude;
    double longitude;
    std::chrono::system_clock::time_point recordTime;
    std::string pollutant;
    double concentration;
    std::string pollutantUnit;
    double rawConcentration;
    int aqi;
    int category;
    std::string siteName;
    std::string siteAgency;
    std::string aqsId;
    std::string countryCodeAqsId;

public:
    Record();
    Record(const double latitude, const double longitude, const std::chrono::system_clock::time_point& recordTime,
    const std::string& pollutant, const double concentration, const std::string& pollutantUnit,
    double rawConcentration, int aqi, int category, const std::string& siteName, const std::string& siteAgency,
    const std::string& aqsId, const std::string& countryCodeAqsId);
    
    // Getter methods
    double getLatitude() const;
    double getLongitude() const;
    std::chrono::system_clock::time_point getRecordTime() const;
    std::string getPollutant() const;
    double getConcentration() const;
    std::string getPollutantUnit() const;
    double getRawConcentration() const;
    int getAQI() const;
    int getCategory() const;
    std::string getSiteName() const;
    std::string getSiteAgency() const;
    std::string getAQSId() const;
    std::string getCountryCodeAQSId() const;

    // Setter methods
    void setLatitude(double lat);
    void setLongitude(double lon);
    void setRecordTime(const std::chrono::system_clock::time_point& time);
    void setPollutant(const std::string& poll);
    void setConcentration(double conc);
    void setPollutantUnit(const std::string& unit);
    void setRawConcentration(double raw_conc);
    void setAQI(int aqi_val);
    void setCategory(int cat);
    void setSiteName(const std::string& site);
    void setSiteAgency(const std::string& agency);
    void setAQSId(const std::string& aqs);
    void setCountryCodeAQSId(const std::string& country_aqs);
};

#endif
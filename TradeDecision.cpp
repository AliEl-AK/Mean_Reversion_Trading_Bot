#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include "mysql_driver.h"
#include "mysql_connection.h"
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "DataReader.hpp"
#include "IndicatorCalculator.hpp"
#include "AverageLowCalculator.hpp"
#include "PriceData.hpp"
#include "Sql.hpp"
#include "json.hpp"
#include "Trade.hpp"
  
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <chrono>

int main(int argc, char *argv[]){

    if (argc != 8) {
        std::cerr << "Usage: " << argv[0] << " timestamp open high low close" << std::endl;
        return 0;
    }

    std::string datetime_str = argv[1];
    double open = std::stod(argv[2]);
    double high = std::stod(argv[3]);
    double low = std::stod(argv[4]);
    double close = std::stod(argv[5]);
    int period = std::stoi(argv[6]);
    std::string indicatorName = std::string(argv[7]);

    std::string timeframe = std::to_string(period) + "min";

    std::string selectAllOHLC = "SELECT * FROM ohlc;";
    std::string deleteFirstOHLC = "DELETE FROM ohlc ORDER BY id LIMIT 1;";
    std::string deleteFirstIndicators = "DELETE FROM indicators ORDER BY id LIMIT 1;";

    std::ifstream i2("mysql_settings.json");
    nlohmann::json j2;
    i2 >> j2;
    std::string host = j2["host"];
    std::string password = j2["password"];
    std::string user = j2["user"];

    sql::Connection* con = establishConnection(host, user, password);
    std::shared_ptr<std::vector<PriceData>> data = fetchOHLC(con, selectAllOHLC);
    executeQuery(con, deleteFirstOHLC);
    delete con;

    data->erase(data->begin());

    PriceData newData;
    std::tm tm = {};
    std::istringstream ss(datetime_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    newData.datetime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    newData.open = open;
    newData.high = high;
    newData.low = low;
    newData.close = close;

    data->push_back(newData);

    IndicatorInfo myIndicator;
    for (const auto &indicator : indicators)
    {
        if (indicator.name == indicatorName)
        {
            myIndicator = indicator;
            break;
        }
    }

    std::shared_ptr<std::vector<std::vector<double>>> indicatorsData = std::make_shared<std::vector<std::vector<double>>>();

    IndicatorCalculator::SMA(data, myIndicator.period, indicatorsData);
    IndicatorCalculator::EMA(data, myIndicator.period, indicatorsData);

    std::map<std::string, std::vector<std::pair<IndicatorInfo, float>>> averageLows;
    AverageLowCalculator::calculateAverageLow(data, myIndicator, averageLows[timeframe], indicatorsData);

    sql::Connection* con3 = establishConnection(host, user, password);

    char formattedQuery[500];
    sprintf(formattedQuery, "INSERT INTO ohlc (datetime, open, high, low, close) VALUES ('%s', %f, %f, %f, %f)",
            datetime_str.c_str(), data->at(data->size() - 1).open, data->at(data->size() - 1).high,
            data->at(data->size() - 1).low, data->at(data->size() - 1).close);
    
    insertSingleData(con3, formattedQuery);

    executeQuery(con3, deleteFirstIndicators);
    char formattedQuery2[500];
    sprintf(formattedQuery2, "INSERT INTO indicators (SMA, EMA, AverageLow, NumOfCycles) VALUES ('%f', '%f', '%f', '%f')",
            indicatorsData->at(indicatorsData->size() - 1)[0], indicatorsData->at(indicatorsData->size() - 1)[1],
            indicatorsData->at(indicatorsData->size() - 1)[2], indicatorsData->at(indicatorsData->size() - 1)[3]);
    insertSingleData(con3, formattedQuery2);

    delete con3;

    bool decision = Trade::takeTrade(data, indicatorsData, myIndicator.name.find("sma") != std::string::npos);

    if (decision){
        return 1;
    }
    return 0;

}    
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "DataReader.hpp"
#include "IndicatorCalculator.hpp"
#include "AverageLowCalculator.hpp"
#include "PriceData.hpp"
#include "Trade.hpp"
#include "FileWriter.hpp"
#include "Sql.hpp"
#include "json.hpp"

#include "mysql_driver.h"
#include "mysql_connection.h"
#include <cppconn/resultset.h>
#include <cppconn/statement.h>


int main(int argc, char *argv[])
{
    std::ifstream i("trade_settings.json");
    nlohmann::json j;
    i >> j;
    std::string symbol = j["symbol"];
    std::string timeframe = j["timeframe"];
    std::string indicatorName = j["indicatorName"];

    std::ifstream i2("mysql_settings.json");
    nlohmann::json j2;
    i2 >> j2;
    std::string host = j2["host"];
    std::string password = j2["password"];
    std::string user = j2["user"];

    std::string selectAllOHLC = "SELECT * FROM ohlc;";
    std::string deleteOldTable = "DROP TABLE IF EXISTS indicators;";

    sql::Connection* con = establishConnection(host, user, password);
    std::shared_ptr<std::vector<PriceData>> data = fetchOHLC(con, selectAllOHLC);
    delete con;

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

    std::cout << std::endl;
    std::cout << "Calculated the indicators successfully." << std::endl;

    sql::Connection* con2 = establishConnection(host, user, password);
    executeQuery(con2, deleteOldTable);
    createTable(con2, "indicators");
    insertData(con2, "indicators", indicatorsData);
    delete con2;

    std::cout << std::endl;
    std::cout << "Indicators data inserted in MySQL Server." << std::endl;

    return 0;
}
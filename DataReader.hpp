#ifndef DATA_READER_HPP
#define DATA_READER_HPP

#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "PriceData.hpp"
#include "Trade.hpp"

class DataReader
{
public:
    static std::shared_ptr<std::vector<PriceData>> readData(const std::string &filename)
    {
        std::ifstream file(filename);
        std::string line, word;
        auto data = std::make_shared<std::vector<PriceData>>();

        std::getline(file, line);

        while (std::getline(file, line))
        {
            std::stringstream s(line);
            PriceData pd;

            std::getline(s, word, ',');
            std::istringstream ss(word);
            std::tm tm = {};
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            pd.datetime = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            std::getline(s, word, ',');
            pd.open = std::stod(word);

            std::getline(s, word, ',');
            pd.high = std::stod(word);

            std::getline(s, word, ',');
            pd.low = std::stod(word);

            std::getline(s, word, ',');
            pd.close = std::stod(word);

            data->push_back(pd);
        }

        return data;
    }

    static std::shared_ptr<std::vector<TradingData>> readOutputFile(std::string filename)
    {
        std::ifstream file(filename);
        std::shared_ptr<std::vector<TradingData>> tradingData = std::make_shared<std::vector<TradingData>>();
        std::string line;

        TradingData td;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string field;

            std::getline(iss, td.timeframe, ',');
            std::getline(iss, td.indicator, ',');
            std::getline(iss, field, ',');
            td.averageLow = std::stod(field);
            std::getline(iss, field, ',');
            td.wins = std::stoi(field);
            std::getline(iss, field, ',');
            td.losses = std::stoi(field);
            std::getline(iss, field, ',');
            td.stopLoss = std::stod(field);
            std::getline(iss, field, ',');
            td.takeProfit = std::stod(field);
            std::getline(iss, field, ',');
            td.accountBalanceStrategy = std::stod(field);
            std::getline(iss, field, ',');
            td.monthlyROI = std::stod(field);
            std::getline(iss, field, ',');
            td.accountBalanceHold = std::stod(field);


            tradingData->push_back(td);
        }
        return tradingData;
    }

    static std::map<std::string, std::shared_ptr<std::vector<PriceData>>> readStockData(const std::string &symbol, const std::vector<std::string> &timeframes)
    {
        std::map<std::string, std::shared_ptr<std::vector<PriceData>>> data;

        for (const auto &timeframe : timeframes)
        {
            std::string filename = "Data/" + symbol + "_" + timeframe + ".csv";
            data[timeframe] = DataReader::readData(filename);
        }

        return data;
    }

};

#endif // DATA_READER_HPP

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
#include "Print.hpp"
#include "Trade.hpp"
#include "FileWriter.hpp"


int main(int argc, char* argv[])
{
    if(argc < 5) {
        std::cerr << "Not enough arguments\n";
        return 1;
    }

    std::string symbol = argv[1];

    std::stringstream ss2(argv[2]);
    std::vector<float> stopLoss;
    float temp;
    while (ss2 >> temp)
    {
        stopLoss.push_back(temp);
        if (ss2.peek() == ',')
            ss2.ignore();
    }

    std::stringstream ss3(argv[3]);
    std::vector<float> ratio;
    while (ss3 >> temp)
    {
        ratio.push_back(temp);
        if (ss3.peek() == ',')
            ss3.ignore();
    }

    float leverage = std::stof(argv[4]);

    std::vector<std::string> period = {"1day", "1h", "30min", "15min"};
    std::map<std::string, std::shared_ptr<std::vector<PriceData>>> timeframes = DataReader::readStockData(symbol, period);
    std::shared_ptr<std::vector<std::vector<double>>> indicatorsData = std::make_shared<std::vector<std::vector<double>>>();

    std::cout << "Calculating SMA and EMA..." << std::endl;

    for (auto &&timeframe : timeframes)
    {
        for (auto &&period : periods)
        {
            IndicatorCalculator::SMA(timeframe.second, period, indicatorsData);
            IndicatorCalculator::EMA(timeframe.second, period, indicatorsData);
        }
    }

    std::cout << "Calculating SMA and EMA : Done!" << std::endl;

    std::cout << "Calculating Average Low..." << std::endl;

    std::map<std::string, std::vector<std::pair<IndicatorInfo, float>>> averageLows;

    for (auto &&timeframe : timeframes)
    {
        for (auto &&indicator : indicators)
        {
            AverageLowCalculator::calculateAverageLow(timeframe.second, indicator, averageLows[timeframe.first], indicatorsData);
        }
    }

    std::cout << "Calculating Average Low : Done!" << std::endl;

    std::cout << "Trading..." << std::endl;

    std::shared_ptr<std::vector<std::vector<std::string>>> logs = std::make_shared<std::vector<std::vector<std::string>>>();
    std::string outputFileName = "Results/output.csv";
    std::ofstream outputFile(outputFileName);  

    int total = timeframes.size() * indicators.size() * stopLoss.size() * ratio.size();
    int done = 0;  

    for (auto &&timeframe : timeframes)
    {
        std::ofstream specificFile("Results/" + timeframe.first + "_trades.csv");
        std::vector<std::pair<IndicatorInfo, float>> averageLow = averageLows[timeframe.first];
        specificFile << timeframe.first << std::endl;

        for (auto &&indicator : averageLow)
        {
            float accountBalance = 1000;
            float riskPerTrade = 0.01;
            IndicatorInfo info = indicator.first;
            float average = indicator.second;
            for(float stop : stopLoss){
                for(float rat :ratio){
                    float takeProfit = stop * rat;
                    Print<int>::displayProgressBar(++done, total);
                    std::pair<std::vector<float>, std::map<std::string, int>> result = Trade::executeTrade(timeframe, info, average, logs, accountBalance, leverage, stop, takeProfit, rat, riskPerTrade);
                    
                    std::vector<float> accountAndRoi = result.first;
                    std::map<std::string, int> winloss = result.second;


                    FileWriter::writeTradesToFile(timeframe.first, info.name, average, specificFile, logs, stop, takeProfit);
                    FileWriter::writeResultToFile(timeframe.first, info.name, average, outputFile, winloss, accountAndRoi, stop, takeProfit);
                }
            }
        }

        specificFile.close();
    }

    outputFile.close();

    std::cout << "Trading : Done!" << std::endl;

    std::cout << "Sorting data..." << std::endl;

    std::shared_ptr<std::vector<TradingData>> tradingData = DataReader::readOutputFile(outputFileName);
    std::sort(tradingData->begin(), tradingData->end(), Trade::compareTradingData);
    FileWriter::writeSortedResults(outputFileName, tradingData);
    
    std::cout << "Sorting data : Done!" << std::endl;

    return 0;
}
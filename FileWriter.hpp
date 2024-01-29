#ifndef FILE_WRITER_HPP
#define FILE_WRITER_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include "PriceData.hpp"

class FileWriter
{
public:

    static void writeTradesToFile(std::string timeframe, std::string indicatorName, float average, std::ofstream &outputFile,
                           std::shared_ptr<std::vector<std::vector<std::string>>> logs, float stopLoss, float takeProfit)
    {
        outputFile << indicatorName << " Stop Loss: " << stopLoss << " Take Profit: "<< takeProfit  << std::endl;
        outputFile << "Win/Loss "
                   <<",Entry Time"
                   <<",Entry Price"
                   <<",Exit Time"
                   << ",Exit Price"
                   << ",Take Profit"
                   << ",Stop Loss"
                   << ",Acccount Balance : 1000"
                   << ",Position Size" 
                   << std::endl;
        for (const auto &log : *logs)
        {
            for (const auto &entry : log)
            {
                outputFile << entry << std::endl;
            }
        }

        outputFile << std::endl
                   << std::endl;

        logs->clear();
    }


    static void writeResultToFile(std::string timeframe, std::string indicatorName, float average, std::ofstream &outputFile,
                           std::map<std::string, int> winLossCount, std::vector<float> accountAndRoi, float stopLoss, float takeProfit)
    {
        outputFile << timeframe << "," << indicatorName << "," << average << "," << winLossCount["win"] << "," << winLossCount["loss"] << "," << stopLoss << "," 
        << takeProfit << "," << accountAndRoi[0] << "," << accountAndRoi[1] * 100 << std::endl;
    }

    static void writeSortedResults(std::string filename, std::shared_ptr<std::vector<TradingData>> tradingData){
        std::ofstream file(filename);
        
        file << "Timeframe"
               << ","
               << "Indicator"
               << ","
               << "Average Low"
               << ","
               << "Wins"
               << ","
               << "Losses"
               << ","
               << "Stop Loss Percentage"
                << ","
                << "Take Profit Percentage"
                << ","
               << "Account Balance Using The Strategy"
               << ","
               << "Monthly ROI"
               << std::endl;

        for (const auto& td : *tradingData) {
            file << td.timeframe << ","
                << td.indicator << ","
                << td.averageLow << ","
                << td.wins << ","
                << td.losses << ","
                << td.stopLoss << ","
                << td.takeProfit << ","
                << td.accountBalanceStrategy << ","
                << td.monthlyROI << "\n";
        }

        file.close();
    }

};

#endif // FILE_WRITER_HPP
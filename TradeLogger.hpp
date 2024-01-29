#ifndef TRADE_LOGGER_HPP
#define TRADE_LOGGER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>

class TradeLogger
{
public:
    static void logTrade(std::string action, double entryPrice, std::string datetimeBegin, double exitPrice, double takeProfit,
                        double stopLoss, std::string datetimeEnd, std::map<std::string, int> &winLossCount, float accountBalance, float positionSize,
                  std::shared_ptr<std::vector<std::vector<std::string>>> logs)
{
    std::stringstream ss;

    ss << (action == "exitWin" ? "WIN" : "LOSS") << ", " << datetimeBegin << ", " << entryPrice 
    <<  ", " << datetimeEnd << "," << exitPrice << ", " << takeProfit << ", " << stopLoss << ", " << accountBalance << ", " << positionSize;

        if (logs->empty())
        {
            logs->push_back(std::vector<std::string>());
        }

        (*logs)[logs->size() - 1].push_back(ss.str());
        winLossCount[action == "exitWin" ? "win" : "loss"]++;
    }
};

#endif // TRADE_LOGGER_HPP

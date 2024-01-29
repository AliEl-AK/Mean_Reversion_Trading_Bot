#ifndef TRADE_HPP
#define TRADE_HPP

#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include "PriceData.hpp"
#include "IndicatorCalculator.hpp"
#include "TradeLogger.hpp"
#include <iomanip>

struct TradingData
{
    std::string timeframe;
    std::string indicator;
    double averageLow;
    int wins;
    int losses;
    double stopLoss;
    double takeProfit;
    double accountBalanceStrategy;
    double monthlyROI;
    double accountBalanceHold;
};

class Trade
{
public:
    static bool compareTradingData(const TradingData &td1, const TradingData &td2)
    {
        return td1.monthlyROI > td2.monthlyROI;
    }

    static bool cooldown(std::chrono::system_clock::time_point &lastTrade, std::chrono::system_clock::time_point &currentTrade, int cooldown)
    {
        std::chrono::duration<double, std::ratio<60 * 60>> timeBetweenTrades = currentTrade - lastTrade;
        if (timeBetweenTrades.count() >= cooldown)
        {
            return true;
        }
        return false;
    }

    static double calculateFees(double positionSize, double entryPrice, double exitPrice)
    {
        double limitFees = 0.000200;
        double marketFees = 0.000550;
        double quantity = positionSize / entryPrice;
        double entryFees = quantity * entryPrice * marketFees;
        double exitFees = quantity * exitPrice * limitFees;
        return entryFees + exitFees;
    }

    static void checkTrade(std::shared_ptr<std::vector<PriceData>> data, IndicatorInfo info, bool &inLongPosition,
                           float &takeProfitPercentage, float &stopLossPercentage,
                           std::map<std::string, int> &winLossCount, std::shared_ptr<std::vector<std::vector<std::string>>> logs, float &accountBalance,
                           float average, float leverage, double ratio, double riskPerTrade)
    {
        float entryPrice;
        float takeProfit;
        float stopLoss;
        int tradesToday = 0;
        std::string entryTime;
        std::string exitTime;
        std::chrono::system_clock::time_point lastTrade;
        std::string currentDay;

        for (int i = 0; i < data->size() - 1; i++)
        {
            std::chrono::system_clock::time_point timeNow = data->at(i).datetime;
            std::time_t timeNow_t = std::chrono::system_clock::to_time_t(timeNow);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&timeNow_t), "%Y-%m-%d %H:%M:%S");
            std::string timeNow_str = ss.str();

            std::string dayNow = timeNow_str.substr(0, 10);

            float currentPrice = data->at(i).close;
            float high = data->at(i).high;
            float low = data->at(i).low;
            float nextOpen = data->at(i + 1).open;
            float positionSize = leverage * accountBalance;

            if (i < info.period)
            {
                continue;
            }

            if (dayNow != currentDay)
            {
                currentDay = dayNow;
                tradesToday = 0;
            }

            float maValue = IndicatorCalculator::getIndicatorValue(data, i, info.period, info.name.find("sma") != std::string::npos);
            float percentage = (maValue - low) / maValue;

            if (!inLongPosition && percentage >= average)
            {
                if (!cooldown(lastTrade, timeNow, 6) || tradesToday >= 2)
                {
                    continue;
                }

                entryPrice = nextOpen;
                entryTime = timeNow_str;
                std::tie(takeProfit, stopLoss) = enterLong(entryPrice, takeProfitPercentage, stopLossPercentage);

                inLongPosition = true;
                tradesToday++;
            }

            else if (inLongPosition)
            {
                if (high >= takeProfit || low <= stopLoss)
                {
                    double exitPrice = high >= takeProfit ? takeProfit : stopLoss;
                    double fees = calculateFees(positionSize, entryPrice, exitPrice);

                    double profit = high >= takeProfit ? positionSize * riskPerTrade * ratio  : -positionSize * riskPerTrade ;
                    accountBalance += (profit - fees);

                    if (accountBalance < 0)
                    {
                        accountBalance = 0;
                    }

                    exitTime = timeNow_str;

                    exitLong(entryPrice, entryTime, takeProfit, stopLoss, exitPrice, exitTime, winLossCount, logs, accountBalance, positionSize);
                    inLongPosition = false;
                    lastTrade = timeNow;
                }
            }
        }
    }

    static std::pair<float, float> enterLong(double entryPrice, float takeProfitPercentage, float stopLossPercentage)
    {
        float stopLossPrice = entryPrice - (entryPrice * stopLossPercentage);

        float takeProfitPrice = entryPrice + (entryPrice * takeProfitPercentage);

        return {takeProfitPrice, stopLossPrice};
    }

    static std::pair<std::vector<float>, std::map<std::string, int>> executeTrade(std::pair<std::string, std::shared_ptr<std::vector<PriceData>>> timeframe,
                                                                                  IndicatorInfo info, float average, std::shared_ptr<std::vector<std::vector<std::string>>> logs,
                                                                                  float accountBalance, float leverage, float stopLossPercentage, float takeProfitPercentage, double ratio, double riskPerTrade)
    {
        bool inLongPosition = false;

        int winCount = 0;
        int lossCount = 0;
        std::map<std::string, int> winLossCount = {{"win", winCount}, {"loss", lossCount}};

        float initialBalance = accountBalance;

        std::shared_ptr<std::vector<PriceData>> data = timeframe.second;

        checkTrade(data, info, inLongPosition, takeProfitPercentage, stopLossPercentage, winLossCount, logs, accountBalance, average, leverage, ratio, riskPerTrade);

        auto cumulTime = calculateCumulativeTime(*data);
        float roi = (accountBalance - initialBalance) / initialBalance;
        float monthlyRoi = roi / cumulTime.count();

        std::vector<float> accountAndRoi = {accountBalance, monthlyRoi};

        return {accountAndRoi, winLossCount};
    }

    static void exitLong(double entryPrice, std::string datetimeBegin, double takeProfit, double stopLoss, double exitPrice,
                         std::string datetimeEnd, std::map<std::string, int> &winLossCount, std::shared_ptr<std::vector<std::vector<std::string>>> logs,
                         float accountBalance, float positionSize)
    {
        TradeLogger::logTrade((exitPrice > entryPrice) ? "exitWin" : "exitLoss", entryPrice, datetimeBegin, (exitPrice > entryPrice) ? takeProfit : stopLoss, takeProfit, stopLoss,
                              datetimeEnd, winLossCount, accountBalance, positionSize, logs);
    }

    static std::chrono::duration<double, std::ratio<60 * 60 * 24 * 30>> calculateCumulativeTime(const std::vector<PriceData> &data)
    {
        std::chrono::system_clock::time_point datetimeBegin = data.at(0).datetime;
        std::chrono::system_clock::time_point datetimeEnd = data.at(data.size() - 1).datetime;
        std::chrono::duration<double, std::ratio<60 * 60 * 24 * 30>> cumulTime = datetimeEnd - datetimeBegin;
        return cumulTime;
    }

    static bool takeTrade(std::shared_ptr<std::vector<PriceData>> &data, std::shared_ptr<std::vector<std::vector<double>>> &indicatorsData, bool isSMA){
        auto price = data->at(data->size() - 1);
        auto indicator = indicatorsData->at(indicatorsData->size() - 1);
        if (indicator[2] > 0.001){
            auto smaPercentage = (indicator[0] - price.close) / indicator[0];
            auto emaPercentage = (indicator[1] - price.close) / indicator[1];
            if(isSMA &&  smaPercentage > indicator[2]){
                std::cout << "sma percentage: " << smaPercentage << " average low: " << indicator[2] << std::endl;
                return true;
            }
            else if(!isSMA && emaPercentage > indicator[2]){
                return true;
            }
        }
        return false;
    }
};

#endif // TRADE_HPP

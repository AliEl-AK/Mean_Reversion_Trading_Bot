#ifndef INDICATOR_CALCULATOR_HPP
#define INDICATOR_CALCULATOR_HPP

#include <memory>
#include <vector>

#include "PriceData.hpp"
#include <limits>

struct IndicatorInfo {
    std::string name;
    int period;
    Indicator indicator;
};

std::vector<int> periods = {21, 50, 100, 200};

std::vector<IndicatorInfo> indicators = {
    {"sma21", periods[0], SMA21},
    {"sma50", periods[1], SMA50},
    {"sma100", periods[2], SMA100},
    {"sma200", periods[3], SMA200},
    {"ema21", periods[0], EMA21},
    {"ema50", periods[1], EMA50},
    {"ema100", periods[2], EMA100},
    {"ema200", periods[3], EMA200}
};

class IndicatorCalculator {
public:

    static void setIndicatorValue(std::shared_ptr<std::vector<PriceData> > &data, int i, int period, double value, bool isSMA) {
    switch (period)
    {
    case 21:
        data->at(i).indicators[isSMA ? SMA21 : EMA21] = value;
        break;
    case 50:
        data->at(i).indicators[isSMA ? SMA50 : EMA50] = value;
        break;
    case 100:
        data->at(i).indicators[isSMA ? SMA100 : EMA100] = value;
        break;
    case 200:
        data->at(i).indicators[isSMA ? SMA200 : EMA200] = value;
        break;
    default:
        break;
    }
    }

    static float getIndicatorValue(std::shared_ptr<std::vector<PriceData>> &data, int i, int period, bool isSMA) {
        switch (period)
        {
        case 21:
            return data->at(i).indicators[isSMA ? SMA21 : EMA21];
        case 50:
            return data->at(i).indicators[isSMA ? SMA50 : EMA50];
        case 100:
            return data->at(i).indicators[isSMA ? SMA100 : EMA100];
        case 200:
            return data->at(i).indicators[isSMA ? SMA200 : EMA200];
        default:
            return std::numeric_limits<float>::quiet_NaN();
        }
    }


    static void SMA(std::shared_ptr<std::vector<PriceData>> &data, int period, std::shared_ptr<std::vector<std::vector<double>>> &indicatorsData){
        double sum = 0;
        for (int i = 0; i < data->size(); i++)
        {
            indicatorsData->push_back(std::vector<double>());
            if (i < period)
            {
                sum += data->at(i).close;
            }
            else
            {
                sum -= data->at(i - period).close;
                sum += data->at(i).close;
            }
            double smaValue = (i < period) ? 0 : (sum / period);
            setIndicatorValue(data, i, period, (i < period) ? 0 : (sum / period), true);
            indicatorsData->back().push_back(smaValue);
        }
    }

    static void EMA(std::shared_ptr<std::vector<PriceData>> &data, int period, std::shared_ptr<std::vector<std::vector<double>>> &indicatorsData){
        float alpha = 2.0 / (period + 1);
        float sum = 0;

        for (int i = 0; i < data->size(); i++)
        {
            double currentClose = data->at(i).close;
            
            if (i < period)
            {
                sum += currentClose;
                if (i == period - 1)
                {
                    float initialSMA = sum / period;
                    setIndicatorValue(data, i, period, initialSMA, false);
                    indicatorsData->at(i).push_back(initialSMA);
                }
            }
            else
            {
                sum -= data->at(i - period).close;
                sum += currentClose;

                float oldEma = getIndicatorValue(data, i - 1, period, false);
                float emaValue = (currentClose - oldEma) * alpha + oldEma;
                setIndicatorValue(data, i, period, emaValue, false);
                indicatorsData->at(i).push_back(emaValue);
            }
        }
    }
};

#endif // INDICATOR_CALCULATOR_HPP

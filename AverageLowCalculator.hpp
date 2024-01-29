#ifndef AVERAGE_LOW_CALCULATOR_HPP
#define AVERAGE_LOW_CALCULATOR_HPP

#include <memory>
#include <vector>
#include <string>
#include <numeric>
#include "IndicatorCalculator.hpp"
#include "PriceData.hpp"
#include <iostream>

class AverageLowCalculator
{
public:
    static void calculateAverageLow(std::shared_ptr<std::vector<PriceData>> &data, IndicatorInfo indicator, 
    std::vector<std::pair<IndicatorInfo, float>> &averageLows, std::shared_ptr<std::vector<std::vector<double>>> &indicatorsData)
    {
        float maximumPercentage = 0;
        std::vector<float> minimums;
        bool underSMA = false;
        int cyclesUnderSMA = 0;
        for (int i = 0; i < data->size(); i++)
        {
            double CurrentLow = data->at(i).close;
            if (i >= indicator.period)
            {
                float indicatorValue = IndicatorCalculator::getIndicatorValue(data, i, indicator.period, indicator.name.find("sma") != std::string::npos);

                if (CurrentLow < indicatorValue)
                {
                    float percentage = (indicatorValue - CurrentLow) / indicatorValue;
                    maximumPercentage = percentage > maximumPercentage ? percentage : maximumPercentage;
                    underSMA = true;
                }
                else if (underSMA)
                {
                    if (maximumPercentage > 0.001)
                    {
                        minimums.push_back(maximumPercentage);
                    }
                    maximumPercentage = 0;
                    cyclesUnderSMA++;
                    underSMA = false;
                }
            }
            float currentAverage = minimums.empty() ? 0 : std::accumulate(minimums.begin(), minimums.end(), 0.0) / minimums.size();
            indicatorsData->at(i).push_back(currentAverage);
            indicatorsData->at(i).push_back(static_cast<double>(minimums.size()));

        }        float average = std::accumulate(minimums.begin(), minimums.end(), 0.0) / minimums.size();
        averageLows.push_back({indicator, average});

    }
};

#endif // AVERAGE_LOW_CALCULATOR_HPP

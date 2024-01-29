#ifndef PRICE_DATA_HPP
#define PRICE_DATA_HPP

#include <map>
#include <chrono>

enum Indicator {
    SMA21,
    SMA50,
    SMA100,
    SMA200,
    EMA21,
    EMA50,
    EMA100,
    EMA200
};

struct PriceData {
    int id;
    std::chrono::system_clock::time_point datetime;
    double open;
    double high;
    double low;
    double close;
    std::map<Indicator, double> indicators;
};

#endif // PRICE_DATA_HPP

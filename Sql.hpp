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

#include "mysql_driver.h"
#include "mysql_connection.h"
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

sql::Connection* establishConnection(const std::string &host, const std::string &user, const std::string &password)
{
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;

    try
    {
        driver = sql::mysql::get_mysql_driver_instance();

        con = driver->connect(host, user, password);

        con->setSchema("trading_data");
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "MySQL Exception: " << e.what() << std::endl;
        return nullptr;
    }

    return con;
}

std::shared_ptr<std::vector<PriceData>> fetchOHLC(sql::Connection* con, const std::string &query)
{
    auto data = std::make_shared<std::vector<PriceData>>();

    try
    {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());

        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

        while (res->next())
        {
            PriceData pd;
            pd.id = res->getInt("id");

            std::string datetime_str = res->getString("datetime");
            std::tm tm = {};
            std::istringstream ss(datetime_str);
            ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
            pd.datetime = std::chrono::system_clock::from_time_t(std::mktime(&tm));

            pd.open = res->getDouble("open");
            pd.high = res->getDouble("high");
            pd.low = res->getDouble("low");
            pd.close = res->getDouble("close");

            data->push_back(pd);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "MySQL Exception: " << e.what() << std::endl;
    }

    return data;
}

std::shared_ptr<std::vector<std::vector<double>>> fetchIndicators(sql::Connection* con, const std::string &query)
{
    std::shared_ptr<std::vector<std::vector<double>>> indicatorsData = std::make_shared<std::vector<std::vector<double>>>();

    try
    {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());

        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(query));

        while (res->next())
        {
            std::vector<double> indicators;
            indicators.push_back(res->getInt("id"));
            indicators.push_back(res->getDouble("SMA"));
            indicators.push_back(res->getDouble("EMA"));
            indicators.push_back(res->getDouble("AverageLow"));
            indicators.push_back(res->getDouble("NumOfCycles"));

            indicatorsData->push_back(indicators);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "MySQL Exception: " << e.what() << std::endl;
    }

    return indicatorsData;
}

void createTable(sql::Connection* con, const std::string &table)
{
    try
    {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());

        std::stringstream query;
        query << "CREATE TABLE IF NOT EXISTS " << table << " ("
              << "id INT AUTO_INCREMENT PRIMARY KEY, "
              << "SMA DOUBLE, "
              << "EMA DOUBLE, "
              << "AverageLow DOUBLE, "
              << "NumOfCycles DOUBLE)";

        stmt->execute(query.str());
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "MySQL Exception: " << e.what() << std::endl;
    }
}

void insertData(sql::Connection* con, const std::string &table, const std::shared_ptr<std::vector<std::vector<double>>>& data)
{
    int done = 0;
    int total = data->size();
    
    try
    {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());

        for (auto &&i : *data)
        {
            std::stringstream query;
            query << "INSERT INTO " << table << " (SMA, EMA, AverageLow, NumOfCycles) VALUES ("
                    << "'" << i[0] << "', "
                    << "'" << i[1] << "', "
                    << "'" << i[2] << "', "
                    << "'" << i[3] << "')";

            stmt->execute(query.str());
            Print<int>::displayProgressBar(++done, total);
        }
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "MySQL Exception: " << e.what() << std::endl;
    }
}

void executeQuery(sql::Connection* con, const std::string &query)
{
    try
    {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());

        stmt->execute(query);
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "MySQL Exception: " << e.what() << std::endl;
    }
}

void insertSingleData(sql::Connection* con, const std::string &query)
{
    try
    {
        std::unique_ptr<sql::Statement> stmt(con->createStatement());
        stmt->execute(query);
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "MySQL Exception: " << e.what() << std::endl;
    }
}


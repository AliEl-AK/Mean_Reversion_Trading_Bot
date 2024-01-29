#!/bin/bash

export DYLD_LIBRARY_PATH=/usr/local/mysql-connector-c++/lib64:$DYLD_LIBRARY_PATH 
# Compile C++ files
g++ -o DataFill DataFill.cpp -I /usr/local/mysql-connector-c++/include/jdbc -L /usr/local/mysql-connector-c++/lib64 -lmysqlcppconn -std=c++11
g++ -o TradeDecision TradeDecision.cpp -I /usr/local/mysql-connector-c++/include/jdbc -L /usr/local/mysql-connector-c++/lib64 -lmysqlcppconn -std=c++11

# Make them executable
chmod +x DataFill
chmod +x TradeDecision

# Run Python script
python3 Bot.py

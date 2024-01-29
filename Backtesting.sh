#!/bin/bash

# Compile C++ files
g++ -o Backtesting Backtesting.cpp -std=c++11

# Make them executable
chmod +x Backtesting

# Run Python script
python3 Backtesting.py
# Moving Average Trading Bot
## Introduction
Welcome to the Moving Average Trading Bot! This trading bot is designed to analyze historical price data, implement a moving average strategy, and conduct backtesting to assess 
the performance of the strategy under different settings. By utilizing the Bybit API for real-time data and MySQL server for data storage, this bot provides a flexible and comprehensive 
tool for traders interested in moving average strategies.

## Bot Overview
### Data Retrieval and Storage
The bot begins by connecting to the Bybit API to fetch historical price data. The retrieved data is then efficiently stored in a MySQL server for quick access and analysis. 
This setup ensures that the bot has access to a reliable and organized dataset to perform its calculations.

### Moving Average Strategy
The core of the bot's trading strategy is based on the moving average indicator. Specifically, the bot calculates the average low under the indicator. 
This information is crucial for identifying potential trade signals. The bot then patiently waits for the next closing price to determine whether to execute a trade based 
on the moving average strategy.

### Trade Decision
The bot patiently awaits the next closing price to make trade decisions. Depending on the calculated moving average and other relevant parameters, the bot determines whether to execute a trade.

## Backtesting
The bot provides a comprehensive backtesting feature, allowing users to assess the effectiveness of the moving average strategy under various settings.

### User Input
The backtesting module prompts users for specific inputs, including the stock symbol, stop loss, ratio, and leverage. These parameters enable users to customize the backtesting process 
to match their trading preferences.

### Result Analysis
After executing backtesting with the user-provided parameters, the bot generates a detailed report on the performance of the moving average strategy. 
The results are then written to a file for easy reference and analysis.

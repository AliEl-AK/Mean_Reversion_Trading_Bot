import time
from Sql import SQL
from Trading import Trade
from Keys import api_key, secret_key, UID


import subprocess
from datetime import datetime
import json

class Color:
    RESET = '\033[0m'
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    PURPLE = '\033[95m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'

def main():
    with open('trade_settings.json', 'r') as f:
        settings = json.load(f)

    symbol = settings['symbol']
    interval = settings['interval']
    period = settings['period']
    indicator_name = settings['indicatorName']
    leverage = settings['leverage']
    stop_loss_percentage = settings['stop_loss_percentage']
    take_profit_percentage = settings['take_profit_percentage']

    print("\n")
    print(f"{Color.BLUE}Moving Average Trading Bot{Color.RESET}")
    print("\n")

    sql_instance = SQL()
    sql_instance.insert_ohlc_sql(symbol, interval)

    subprocess.run(["./DataFill"])  

    print("\n")
    print(f"{Color.GREEN}~Ready{Color.RESET}")

    while True:

        trade_instance = Trade(api_key, secret_key, UID)
        current_data = trade_instance.get_current_data("BTCUSDT", interval)

        timestamp_ms = int(current_data[0])
        timestamp_s = timestamp_ms / 1000.0
        dt_object = datetime.utcfromtimestamp(timestamp_s)
        datetime_str = dt_object.strftime("%Y-%m-%d %H:%M:%S")
        current_data[0] = datetime_str
        print(datetime_str)

        args = ["./TradeDecision"] + current_data + [period, indicator_name]  

        completed_process = subprocess.run(args)

        return_code = completed_process.returncode

        if return_code == 1:
             trade_instance.trade(leverage, current_data[1], symbol, stop_loss_percentage, take_profit_percentage)
            
        time.sleep(60)

if __name__ == "__main__":
    main()
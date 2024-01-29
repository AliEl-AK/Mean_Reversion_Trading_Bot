from datetime import datetime
import requests
import datetime
import json
import time
from requests.exceptions import RequestException
import requests
import json
import time
import ccxt
import csv
import os

one_minute = 60
one_year_in_ms = 31499999999
ms_in_s = 1000

class Trade:
    def __init__(self, api_key, secret_key, UID):
        self.exchange = ccxt.bybit({
            'apiKey': api_key,
            'secret': secret_key,
            'uid': UID,
        })

    def write_to_csv(self, open_time, open_price, close_time, close_price, leverage, fees,  filename='output.csv'):
        roi = (close_price - open_price) / open_price * leverage * 100
        file_exists = os.path.isfile(filename)
        with open(filename, 'a', newline='') as file:
            writer = csv.writer(file)
            if not file_exists:
                writer.writerow(['Open Time', 'Open Price', 'Close Time', 'Close Price', 'ROI', 'Fees'])  # column names
            writer.writerow([open_time, open_price, close_time, close_price, roi, fees])

    def determine_position_size(self, account_balance, leverage, last_open_price):
        position_size_usd =  account_balance * leverage
        position_size = position_size_usd / last_open_price
        #return round(position_size, 3)
        return account_balance

    def get_wallet_balance(self):
        balance = self.exchange.fetch_balance()['info']['result']['list'][0]['totalWalletBalance']
        return float(balance)

    def determine_sl_tp(self, entry_price, stop_loss_percentage, take_profit_percentage):
        stop_loss = entry_price * (1-stop_loss_percentage)
        take_profit = entry_price * (1+take_profit_percentage)
        return stop_loss, take_profit

    def current_positions(self):
        return self.exchange.fetch_positions()

    def get_positions(self):
        open_trade = self.exchange.fetch_my_trades()[-2]
        open_time = datetime.utcfromtimestamp(open_trade['timestamp'] / 1000.0).strftime('%Y-%m-%d %H:%M:%S')
        open_price = open_trade['price']
        close_trade = self.exchange.fetch_my_trades()[-1]
        close_time = datetime.utcfromtimestamp(close_trade['timestamp'] / 1000.0).strftime('%Y-%m-%d %H:%M:%S')
        close_price = close_trade['price']
        close_fee = close_trade['fee']['cost']
        self.write_to_csv(open_time, open_price, close_time, close_price, 10, close_fee)

    def enter_long(self, symbol, position_size, stop_loss, take_profit):
        self.exchange.create_order_with_take_profit_and_stop_loss(symbol, 'market', 'buy', position_size, 0, take_profit, stop_loss)

    def trade(self, leverage, last_open_price, symbol, stop_loss_percentage, take_profit_percentage):
        balance = self.get_wallet_balance()
        print('\n')
        print(f"Balance: {balance}")
        position_size = self.determine_position_size(balance, leverage, last_open_price)
        stop_loss, take_profit = self.determine_sl_tp(last_open_price, stop_loss_percentage, take_profit_percentage)
        print(f"Position size: {position_size} BTC")
        print(f"Leverage: {leverage}")
        print(f"Stop loss: {stop_loss}")
        print(f"Take profit: {take_profit}")
        self.enter_long(symbol, position_size, stop_loss, take_profit)
        print("Trade executed")
        while True:
            pos = self.current_positions()
            if pos == []:
                break
        self.get_positions()
        print("Trade closed")
        print('\n')


                

    def get_current_data(self, symbol, interval):
        url2 = f"https://api-testnet.bybit.com/v5/market/kline?category=linear&symbol={symbol}&interval={interval}&limit=1"

        size = one_minute/interval
        time_array = []
        for i in range(int(size)):
            time_array.append(i*interval)
        print(f"Waiting for the next {interval} mins close.")
        while True:
            current_minute = datetime.datetime.now().minute
            if current_minute in time_array:
                time.sleep(10)
                response = requests.get(url2)
                response_json = json.loads(response.text)
                error_code = response_json.get('retCode')
                if error_code != 0:
                    raise RequestException(f"Error {error_code}: {response_json.get('retMsg')}")
                data = response_json.get('result').get('list')[0]
                data = data[0:5]
                timestamp_ms = int(data[0])

                timestamp_s = timestamp_ms / 1000.0

                dt_object = datetime.datetime.utcfromtimestamp(timestamp_s)

                datetime_str = dt_object.strftime("%Y-%m-%d %H:%M:%S")

                return data
import datetime
import json
from sqlalchemy import create_engine
from sqlalchemy.sql import text
from Others import display_progress_bar
from requests.exceptions import RequestException
import requests


one_minute = 60
one_year_in_ms = 31499999999
ms_in_s = 1000

class SQL:
    def __init__(self):
        with open('mysql_settings.json') as f:
            data = json.load(f)

        self.user = data['user']
        self.password = data['password']
        self.database = data['database']
        self.host = data['host']
        self.engine = create_engine(f'mysql+pymysql://{self.user}:{self.password}@{self.host}/{self.database}')

    def create_table(self):
        with self.engine.connect() as connection:
            try:
                drop_table_query = "DROP TABLE IF EXISTS ohlc"
                connection.execute(text(drop_table_query))

                create_table_query = """
                CREATE TABLE ohlc (
                    id INT AUTO_INCREMENT PRIMARY KEY,
                    datetime DATETIME,
                    open DECIMAL(10, 2),
                    high DECIMAL(10, 2),
                    low DECIMAL(10, 2),
                    close DECIMAL(10, 2)
                )
                """
                connection.execute(text(create_table_query))

            except Exception as err:
                print(f"Error: {err}")

    def insert_data(self, sql_data):
        with self.engine.connect() as connection:
            try:
                connection.execute(text("SET time_zone = '+00:00'"))

                for data in sql_data:
                    insert_query = text(f"""
                    INSERT INTO ohlc (datetime, open, high, low, close)
                    VALUES (
                        FROM_UNIXTIME({data[0] / 1000}),
                        {data[1]},
                        {data[2]},
                        {data[3]},
                        {data[4]}
                    )
                    """)
                    connection.execute(insert_query)

                connection.commit()
                print("Data inserted in MySQL Server.")

            except Exception as err:
                print(f"Error: {err}")

    def get_times_array(self, interval):
        now = datetime.datetime.utcnow()

        minutes = (now.minute // interval) * interval
        now = now.replace(minute=minutes, second=0, microsecond=0)
        now_ms = int(now.timestamp() * ms_in_s)

        last_time_ms = now_ms

        times = []
        times.append(now_ms)
        while now_ms - last_time_ms < one_year_in_ms:
            time_ago = now - datetime.timedelta(minutes=interval * ms_in_s)

            minutes = (time_ago.minute // interval) * interval
            time_ago = time_ago.replace(minute=minutes, second=0, microsecond=0)
            time_ago_ms = int(time_ago.timestamp() * ms_in_s)

            times.append(time_ago_ms)

            last_time_ms = time_ago_ms
            now = time_ago

        times.reverse()
        return times
    
    def get_historical_data(self, symbol, interval):
        times = self.get_times_array(interval)
        sql_data = []
        progress = 0
        print("Importing the historical data of Bitcoin.")
        for i, time in enumerate(times[:-1]):
            url2 = f"https://api-testnet.bybit.com/v5/market/kline?category=linear&symbol={symbol}&interval={interval}&start={time+15000}&end={times[i + 1]}&limit=1000"
            response = requests.get(url2)
            response_json = json.loads(response.text)
            error_code = response_json.get('retCode')
            if error_code != 0:
                raise RequestException(f"Error {error_code}: {response_json.get('retMsg')}")
            data = response_json.get('result').get('list')

            data.reverse()
            for i in range(len(data)):
                data[i] = [float(x) for x in data[i]]

            sql_data.extend(data)
            progress+=1
            display_progress_bar(progress, len(times[:-1]))
        return sql_data

    def insert_ohlc_sql(self, symbol, interval):
        sql_data = self.get_historical_data(symbol, interval)
        self.create_table()
        self.insert_data(sql_data)
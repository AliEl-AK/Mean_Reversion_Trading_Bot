import requests
import pandas as pd
import json
from io import StringIO
from datetime import datetime
from requests.exceptions import RequestException
from pandas.errors import EmptyDataError

class DataImporter:
    def __init__(self, symbol, start):
        self.symbol = symbol
        self.start = start

    def display_progress_bar(self, progress, total, width=20):
        percentage = progress / total

        bar_width = int(percentage * width)

        print("[", end="")
        for i in range(width):
            if i < bar_width:
                print("=", end="")
            else:
                print(" ", end="")
        print(f"] {int(percentage * 100.0)}%\r", end="")
        print() if progress == total else print("", end="", flush=True)

    def handle_request_error(self, response):
        error_messages = {
            400: "Bad Request -",
            401: "Invalid API key is used.",
            403: "Forbidden - Cannot do this.",
            404: f"Not Found - Wrong Symbol: {self.symbol}",
            414: "URI Too Long - Wrong parameter.",
            429: "Too Many Requests - Too fast, come back later.",
            500: "Internal Server Error - Contact the API provider."
        }
        response_json = json.loads(response.text)
        error_code = response_json.get('code')
        if error_code != 200:
            raise RequestException(f"Error {error_code}: {error_messages.get(error_code, 'Unknown error')}")


    def import_data(self):
        intervals = ['15min', '30min', '1h', '1day']
        split_date = datetime.strptime('2022-08-25', '%Y-%m-%d')
        progress = 0

        print("Importing the data...")

        for interval in intervals:
            url = f"https://api.twelvedata.com/time_series?apikey=056c816f2be64e36be4737c00ea5b672&interval={interval}&symbol={self.symbol}&start_date={self.start}&format=CSV&dp=8&timezone=utc"
            response = requests.get(url)

            try:
                response.raise_for_status()  

                self.handle_request_error(response)


            except (RequestException, EmptyDataError) as e:
                print(f"{e}. Please try again.")
                return False

            except json.JSONDecodeError:
                data = response.text

                df = pd.read_csv(StringIO(data), sep=';')

                df = df.sort_values(by='datetime', ascending=True)

                if 'volume' in df.columns:
                    df = df.drop('volume', axis=1)

                # Split the data for TSLA stock before the split date
                if interval in ['30min', '1h'] and self.symbol == 'TSLA':
                    df['datetime'] = pd.to_datetime(df['datetime'])
                    df.loc[df['datetime'] < split_date, ['open', 'high', 'low', 'close']] /= 3
                    df[['open', 'high', 'low', 'close']] = df[['open', 'high', 'low', 'close']].round(5)

                if '/' in self.symbol:
                    filename = self.symbol.replace('/', '')
                else:
                    filename = self.symbol

                df.to_csv(f'Data/{filename}_{interval}.csv', index=False)

                progress+=1
                self.display_progress_bar(progress, len(intervals))

        print("Importing the data: Done!")
        return True



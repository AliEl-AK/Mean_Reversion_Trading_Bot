import subprocess
from ImportData import DataImporter
from Cleaner import FolderCleaner

def main():
    data_cleaner = FolderCleaner('Data')
    data_cleaner.clear_folder()

    results_cleaner = FolderCleaner('Results')
    results_cleaner.clear_folder()

    while True:
        symbol = str(input("Enter the symbol of the stock you want to import: ")).upper()
        date = str(input("Do you want to import the data from a specific date (S) or from the beginning of the year (Y): ")).upper()
        if date == 'Y':
            time = '2023-01-01'
        elif date == 'S':
            time = str(input("Enter the date you want to start importing from (YYYY-MM-DD): ")).upper()

        importer = DataImporter(symbol, time)
        if importer.import_data():
            break

    usual_stop_losses = [0.005, 0.01, 0.015, 0.02, 0.025]
    stop_loss_choice = int(input("Do you want to use custom StopLosses (1) or these [0.005, 0.01, 0.015, 0.02, 0.025] (2): "))
    if stop_loss_choice == 1:
        stop_losses = []
        while True:
            stop_loss = float(input("Enter a StopLoss (0.001): "))
            stop_losses.append(stop_loss)
            if str(input("Do you want to add another StopLoss (Y/N): ")).upper() == 'N':
                break
    elif stop_loss_choice == 2:
        stop_losses = usual_stop_losses

    usual_ratio = [1.0, 1.5, 2.0, 2.5, 3.0]
    ratio_choice = int(input("Do you want to use custom ratio (1) or these [1.0, 1.5, 2.0, 2.5, 3.0] (2): "))
    if ratio_choice == 1:
        ratios = []
        while True:
            r = float(input("Enter a ratio: "))
            ratios.append(r)
            if str(input("Do you want to add another Ratio (Y/N): ")).upper() == 'N':
                break
    elif ratio_choice == 2:
        ratios = usual_ratio

    while True:
        leverage_choice = str(input("Do you want to use leverage or not (Y/N): ")).upper()
        if leverage_choice == 'Y':
            leverage = float(input("Enter the leverage you want to use: "))
            break
        elif leverage_choice == 'N':
            leverage = 1
            break
        else:
            print("Invalid input. Please enter Y or N.")

    if '/' in symbol:
        symbol = symbol.replace('/', '')

    subprocess.run(["./backtesting", symbol, ','.join(map(str, stop_losses)), ','.join(map(str, ratios)), str(leverage)])

    print("Finished!")

    csv_file = 'Results/output.csv'

    subprocess.run(('open', csv_file))

if __name__ == "__main__":
     main()
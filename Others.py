def display_progress_bar(progress, total, width=20):
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
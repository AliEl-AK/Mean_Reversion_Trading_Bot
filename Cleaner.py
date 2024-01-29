import os
import shutil

class FolderCleaner:
    def __init__(self, folder):
        self.folder = folder

    def clear_folder(self):
        for filename in os.listdir(self.folder):
            file_path = os.path.join(self.folder, filename)
            try:
                if os.path.isfile(file_path) or os.path.islink(file_path):
                    os.unlink(file_path)
                elif os.path.isdir(file_path):
                    shutil.rmtree(file_path)
            except Exception as e:
                print(f'Failed to delete {file_path}. Reason: {e}')
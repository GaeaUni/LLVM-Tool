import json


class DependecyProcessor:
    def __init__(self, file_path):
        self.file_path = file_path
        self.json_data = json.load(self.file_path)
        self.visited = set()

    def process(self):
        pass

    def get_all_nodes(self):
        return self.json_data.keys().map(lambda x: x.split("/")[-1])

from distutils import command
import json
import os


class PreProcessor:
    def __init__(self, json_file_path, output_file_path):
        self.json_file_path = json_file_path
        self.output_file_path = output_file_path

    def run(self):
        self.__parse_json()
        self.__delete_pods()
        self.__delete_gmodules()
        os.makedirs(os.path.dirname(self.output_file_path), exist_ok=True)
        with open(self.output_file_path, 'w') as f:
            json.dump(self.json_data, f, indent=4)

    def __parse_json(self):
        with open(self.json_file_path) as f:
            self.json_data = json.load(f)

    def __delete_pods(self):
        self.json_data = [
            item for item in self.json_data if not ("/Pods/" in item['file'])]

    def __delete_gmodules(self):
        for item in self.json_data:
            item['command'] = item['command'].replace(' -gmodules', '')

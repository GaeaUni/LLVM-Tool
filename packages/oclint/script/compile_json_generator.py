import json
import commit_file_parser
import sys
import os
from build_setting_parser import BuildSettingParser

workPath = sys.path[0]
compile_commands_json_path = "{0}/../tmp/compile_commands.json".format(
    workPath)


class CompileJsonGenerator:
    def __init__(self, repo_name, project_path):
        self.repo_name = repo_name
        self.project_path = self.get_xcproject_path(repo_name, project_path)
        self.compile_commands_json = []

    def get_xcproject_path(self, repo_name, project_path):
        return "{0}/Pods/{1}.xcodeproj".format(project_path, repo_name)

    def process(self, commit_files):
        self.init_items(commit_files)
        self.add_setting_commands()
        self.add_source_file_command()
        self.add_arc_command()
        self.write_to_tmp()

    def delete_clean_files(self, commit_files):
        with open(compile_commands_json_path, 'r') as f:
            self.compile_commands_json = json.load(f)
        for item in self.compile_commands_json:
            if item["file"] not in commit_files:
                self.compile_commands_json.remove(item)
        self.write_to_tmp()

    def init_items(self, commit_files):
        for commit_file in commit_files:
            if self.is_source_file(commit_file):
                item = self.process_file(commit_file)
                if item:
                    self.compile_commands_json.append(item)

    def write_to_tmp(self):
        os.makedirs(os.path.dirname(compile_commands_json_path), exist_ok=True)
        with open(compile_commands_json_path, 'w+') as f:
            json.dump(self.compile_commands_json, f, indent=4)

    def add_setting_commands(self):
        build_setting_parser = BuildSettingParser(
            self.project_path)
        command = build_setting_parser.process()
        for item in self.compile_commands_json:
            item["command"] += command

    def add_source_file_command(self):
        for item in self.compile_commands_json:
            item["command"] += " -c " + repr(item["file"])

    def add_arc_command(self):
        for item in self.compile_commands_json:
            item["command"] += " -fobjc-arc -fobjc-weak "

    def is_source_file(self, file_path):
        return file_path.endswith(".m") or file_path.endswith(".mm") or file_path.endswith(".h")

    def process_file(self, commit_file):
        item = {}
        item["file"] = commit_file
        item["directory"] = "/"
        item["command"] = "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang "
        return item

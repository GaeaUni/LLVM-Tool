
import argparse
from compile_json_generator import CompileJsonGenerator, compile_commands_json_path
from commit_file_parser import CommitFileParser
import os
import json
from linter import CommitFilesLinter


def need_regenerate_compile_commands_json(commit_files):
    json_files = []
    if os.path.exists(compile_commands_json_path):
        with open(compile_commands_json_path, 'r') as f:
            data = json.load(f)
            for item in data:
                json_files.append(item["file"])
        for item in commit_files:
            if item not in json_files:
                return True
        return False
    else:
        return True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-project_path")
    parser.add_argument("-main_project_path")
    args = parser.parse_args()
    project_path = args.project_path
    main_project_path = args.main_project_path

    if not project_path:
        # 先用自己的项目路径debug
        project_path = "/Volumes/T7/company/hammer-workspace/QMapBusKit"
    if not main_project_path:
        main_project_path = os.path.dirname(project_path)
    project_name = project_path.split("/")[-1]
    commit_files = CommitFileParser(project_path).process()
    generator = CompileJsonGenerator(project_name, main_project_path)
    if need_regenerate_compile_commands_json(commit_files):
        print("generate compile_commands.json")
        generator.process(commit_files)
    else:
        print("delete files not commit")
        generator.delete_clean_files(commit_files)
    linter = CommitFilesLinter()
    linter.process()


if __name__ == "__main__":
    main()

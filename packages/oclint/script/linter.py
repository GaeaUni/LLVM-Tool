
import subprocess

from compile_json_generator import compile_commands_json_path
import os


class CommitFilesLinter:
    def __init__(self):
        pass

    def install(self):
        cmd = "which oclint"
        result = subprocess.run(cmd.split(), encoding='utf-8').returncode
        if result == 0:
            return
        # brew install oclint
        cmd = "brew install oclint"
        result = subprocess.run(cmd.split(), encoding='utf-8').returncode
        if result != 0:
            print("brew install oclint failed")
            assert False
        # brew link oclint
        cmd = "brew link oclint"
        result = subprocess.run(cmd.split(), encoding='utf-8').returncode
        if result != 0:
            print("brew link oclint failed")
            assert False

    def process(self):
        self.install()
        path = os.path.dirname(compile_commands_json_path)
        cmd = "oclint-json-compilation-database -v -- -report-type html -o {0}/oclint.html".format(
            path)

        result = subprocess.run(
            cmd.split(), cwd=path, encoding='utf-8').returncode
        if result >= 6:
            print("OCLint return compile error")
            os.system("open {0}/oclint.html".format(path))
            assert False
        os.system("open {0}/oclint.html".format(path))
        return result

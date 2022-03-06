import subprocess
import installer
import os


class PrecommitInstaller:
    def __init__(self):
        self.install_precommit()
        self.install_git_config_hook()

    def install_git_config_hook(self):
        os.system("git config --global init.templateDir ~/.git-template")
        os.system("pre-commit init-templatedir ~/.git-template")

    def process(self, main_project_path):
        dev_pods = self.find_all_developer_pod_path(main_project_path)
        self.install_precommit_in_dev_pod(dev_pods)
        self.install_precommit_config(dev_pods)

    def install_precommit(self):
        installer.install_brew("pre-commit")

    def install_precommit_in_dev_pod(self, dev_pods):
        for pod_path in dev_pods:
            cmd = "pre-commit install"
            result = subprocess.run(
                cmd.split(), encoding='utf-8', cwd=pod_path, stdout=subprocess.PIPE).returncode
            if result != 0:
                print("pre-commit install failed in {0}".format(pod_path))
                assert False
            else:
                print("pre-commit install success in {0}".format(pod_path))

    def install_precommit_config(self, dev_pod_paths):
        dir = os.path.dirname(__file__)
        template_path = os.path.join(dir, ".pre-commit-config.yaml.template")
        # TODO:考虑devpod不是一个目录的情况
        # entry = os.path.join(dir, "main.py")
        # entry = "python3 {0}".format(entry)
        # with open(template_path, "r") as f:
        #     template = f.read()
        #     template = template.replace("{0}", ",".join(dev_pod_paths))
        for path in dev_pod_paths:
            config_path = os.path.join(path, ".pre-commit-config.yaml")
            cmd = "cp {0} {1}".format(template_path, config_path)
            os.system(cmd)

    def find_all_developer_pod_path(self, main_project_path):
        # TODO:考虑devpod不是一个目录的情况
        os.chdir(main_project_path)
        result = os.popen(
            "find {0} -name '*.podspec' -type f".format(main_project_path)).read()
        result = filter(lambda x: x != "", result.split("\n"))
        result = map(lambda x: os.path.dirname(x), result)
        return list(result)


def main():
    installer = PrecommitInstaller()
    path = os.getcwd()
    print("start precommit installer {0}".format(path))
    installer.process(path)


if __name__ == "__main__":
    main()

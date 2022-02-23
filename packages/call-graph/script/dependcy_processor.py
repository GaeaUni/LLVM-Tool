
import json
import re
from typing import Set
import graphviz
import subprocess
import os
import sys

workPath = sys.path[0]


class DependecyProcessor:
    def __init__(self, file_path, dirty_files):
        self.file_path = file_path
        self.dirty_files = dirty_files
        self.json_data = json.loads(open(file_path).read())
        self.visited = set()
        self.file_dependcys = {}
        self.file_implements = {}
        self.dependcy_files = {}
        self.edges = []

    def process(self):
        files = self.get_all_file_paths()
        for file_path in files:
            self.process_file(file_path)
        self.build_reverse_dependency()
        print("file_dependcys\n")
        print(self.file_dependcys)
        print("file_implements\n")
        print(self.file_implements)
        print("dependcy_files\n")
        print(self.dependcy_files)
        init_nodes = self.get_diff_files()
        self.generate_dependcy_graph(init_nodes)

    def render(self):
        g = graphviz.Digraph(comment='The Module Dependcy Graph')
        g.graph_attr["bgcolor"] = "#FFFFDD"
        g.node_attr["shape"] = "box"
        self.draw(g)
        print(g.source)

    def generate_dependcy_graph(self, file_modules):
        for module in file_modules:
            self.dfs_dependency(module)

    def edge(self, module1, module2):
        self.edges.append((module1, module2))

    def draw(self, g):
        for edge in self.edges:
            g.edge(edge[0], edge[1])
        g.view(directory="{0}/../tmp".format(workPath))

    def dfs_dependency(self, file_module):
        if file_module in self.visited:
            return
        self.visited.add(file_module)

        implements = self.file_implements[file_module]
        for implement in implements:
            dependcy_moduels = self.dependcy_files.get(implement)
            if not dependcy_moduels:
                continue
            for dependcy_module in dependcy_moduels:
                self.edge(dependcy_module, file_module)
                self.dfs_dependency(dependcy_module)

    def get_diff_files(self):
        files = list(
            filter(lambda x: self.is_source_file(x), self.dirty_files))
        file_moduels = set()
        for file in files:
            file_moduels.add(self.get_file_module(file))
        return file_moduels

    def is_source_file(self, file_path):
        return file_path.endswith(".h") or file_path.endswith(".m") or file_path.endswith(".mm")

    def get_file_module(self, file_path):
        return file_path.split("/")[-1].split(".")[0]

    def process_file(self, file_path):
        header_path = re.sub('\.[a-z]+', '.h', file_path)
        file_module = file_path.split("/")[-1].split(".")[0]
        try:
            content = open(header_path).read()
            self.process_header_file(file_module, content)
        except FileNotFoundError:
            return
        self.process_header_file(file_module, content)
        with open(file_path) as f:
            content = f.read()
        self.process_implement_file(file_module, content)

    def get_all_file_paths(self):
        files = []
        for item in self.json_data:
            files.append(item["file"])
        return files

    def process_header_file(self, file_module, content):
        if not self.file_implements.get(file_module):
            self.file_implements[file_module] = set()
        if not self.file_dependcys.get(file_module):
            self.file_dependcys[file_module] = set()
        implementedInterfaces, implementedProtocols, dependcyInterfaces = self.get_declare_objc_interface(
            content)
        self.file_implements[file_module].update(implementedInterfaces)
        self.file_implements[file_module].update(implementedProtocols)
        self.file_dependcys[file_module].update(dependcyInterfaces)
        self.process_parameter_property(file_module, content)

    def process_implement_file(self, file_path, content):
        self.process_parameter_property(file_path, content)
        callers = self.get_objc_call_dependency(content)
        self.file_dependcys[file_path].update(callers)

    def process_parameter_property(self, file_path, content):
        dependcyInterfaces, dependcyProtocols = self.get_objc_property_dependency(
            content)
        self.file_dependcys[file_path].update(dependcyInterfaces)
        self.file_dependcys[file_path].update(dependcyProtocols)

        dependcyInterfaces, dependcyProtocols = self.get_objc_parameter_dependency(
            content)
        self.file_dependcys[file_path].update(dependcyInterfaces)
        self.file_dependcys[file_path].update(dependcyProtocols)

    def build_reverse_dependency(self):
        dependcy_files = {}
        for file_path, dependcy_objects in self.file_dependcys.items():
            for dependcy_object in dependcy_objects:
                if dependcy_object not in dependcy_files:
                    dependcy_files[dependcy_object] = []
                dependcy_files[dependcy_object].append(file_path)
        self.dependcy_files = dependcy_files

    def get_objc_property_dependency(self, content):
        descriptor = "\s*\(.+\)\s*"
        type = "(([A-Z][a-zA-Z0-9_]+)|id)(<[a-zA-Z0-9_]+>)?"
        pat = re.compile(
            "@property\s{0}({1})\s".format(descriptor, type))
        properties = pat.finditer(content)
        dependcyInterfaces = set()
        dependcyProtocols = set()
        for match in properties:
            className = match.group(2)
            protocolName = match.group(4)
            if not className == "id":
                dependcyInterfaces.add(className)
            if protocolName:
                dependcyProtocols.add(protocolName)
        dependcyInterfaces = self.filter_system_object(dependcyInterfaces)
        dependcyProtocols = self.filter_system_object(dependcyProtocols)
        return dependcyInterfaces, dependcyProtocols

    def get_objc_parameter_dependency(self, content):
        type = "(([A-Z][a-zA-Z0-9_]+)|id)(<[a-zA-Z0-9_]+>)?\s*\*?"
        parameter = ":\s*\(({0})\s*\*?\s*\)".format(type)
        pat = re.compile(parameter)
        parameters = pat.finditer(content)

        dependcyInterfaces = set()
        dependcyProtocols = set()
        for match in parameters:
            className = match.group(2)
            protocolName = match.group(4)
            if not className == "id":
                dependcyInterfaces.add(className)
            if protocolName:
                dependcyProtocols.add(protocolName)
        dependcyInterfaces = self.filter_system_object(dependcyInterfaces)
        dependcyProtocols = self.filter_system_object(dependcyProtocols)
        return dependcyInterfaces, dependcyProtocols

    def get_objc_call_dependency(self, content):
        classId = "([A-Z][a-zA-Z0-9_]+)"
        call = "\[{0}\s".format(classId)
        pat = re.compile(call)
        calls = pat.finditer(content)
        callers = set()
        for match in calls:
            callers.add(match.group(1))
        callers = self.filter_system_object(callers)
        return callers

    def get_declare_objc_interface(self, content):
        # 只处理类的声明h文件
        interface = "@interface\s+([a-zA-Z0-9_]+)\s*"
        super = ":\s*([a-zA-Z0-9_]+)\s*"
        protocol = "(<[a-zA-Z0-9_,]+>)?"
        pat = re.compile(
            "{0}{1}{2}".format(interface, super, protocol))
        interfaces = pat.finditer(content)
        implementedInterfaces = []
        implementedProtocols = []
        dependcyInterfaces = []
        for match in interfaces:
            interfaceName = match.group(1)
            superName = match.group(2)
            protocolNames = match.group(3)

            dependcyInterfaces.append(superName)
            implementedInterfaces.append(interfaceName)
            if protocolNames:
                implementedProtocols.extend(protocolNames.split(","))
        dependcyInterfaces = self.filter_system_object(dependcyInterfaces)
        implementedProtocols = self.filter_system_object(implementedProtocols)
        implementedInterfaces = self.filter_system_object(
            implementedInterfaces)
        return implementedInterfaces, implementedProtocols, dependcyInterfaces

    def filter_system_object(self, object_names: Set[str]):
        result = filter(lambda x: not x.startswith(
            "NS") and not x.startswith("UI") and not x.startswith("<UI") and not x.startswith("<NS"), object_names)
        return set(result)

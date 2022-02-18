# -*- coding: utf-8 -*-
import argparse
import os
import json
from diff_parser import DiffParser
from draw_call import DrawCall
from pathlib import Path
import sys

workPath = sys.path[0]


def runClangTooling(diffPath, compileJsonPath):
    cmd = "{0}/../bin/call-graph -p {1} {2}".format(
        workPath, compileJsonPath, diffPath)
    os.system(cmd)


def dumpDiffJson(path, commit1, commit2):
    diffParser = DiffParser()
    diffs = diffParser.parser(commit1, commit2, path)
    diffPath = "{0}/../tmp/diff.json".format(workPath)
    os.makedirs(os.path.dirname(diffPath), exist_ok=True)
    with open(diffPath, 'w') as f:
        f.write(diffs)
    return diffPath


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-project_path")
    parser.add_argument("-compile_json_path")
    parser.add_argument("-commit_hashs", nargs='+')
    args = parser.parse_args()
    project_path = args.project_path
    commit_hashs = args.commit_hashs or ["", ""]
    compile_json_path = args.compile_json_path

    print("参数为 {0}".format(args))
    print("1.dump出diff文件")
    diffsPath = dumpDiffJson(project_path, commit_hashs[0], commit_hashs[1])
    print("2.调用clang产生init_nodes和call_map")
    runClangTooling(diffsPath, compile_json_path)
    print("3.解析init_nodes和call_map产生dot源文件,并且显示")

    initNodes = json.loads(
        open("{0}/../tmp/init_nodes.json".format(workPath)).read())
    callMap = json.loads(
        open("{0}/../tmp/call_map.json".format(workPath)).read())
    drawCall = DrawCall()
    graph = drawCall.generateGraph(initNodes, callMap)
    print(graph.source)
    graph.view(directory="{0}/../tmp".format(workPath))


if __name__ == "__main__":
    main()

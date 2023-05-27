import lldb
import os


def callback():
    debugger = lldb.debugger
    interpreter: lldb.SBCommandInterpreter = debugger.GetCommandInterpreter()
    nums_module = debugger.GetSelectedTarget().GetNumModules()
    # 获取当前的目标（你正在调试的程序）
    target: lldb.SBTarget = debugger.GetSelectedTarget()
    returnObject = lldb.SBCommandReturnObject()
    link_files = dict()
    for i in range(nums_module):
        module = target.GetModuleAtIndex(i)
        nums_unit = module.GetNumCompileUnits()
        for j in range(nums_unit):
            unit: lldb.SBCompileUnit = module.GetCompileUnitAtIndex(j)
            filespec: lldb.SBFileSpec = unit.GetFileSpec()
            num_support_files = unit.GetNumSupportFiles()
            for k in range(num_support_files):
                filespec = unit.GetSupportFileAtIndex(k)
                if filespec.fullpath == None:
                    continue
                is_soft_link = os.path.islink(filespec.fullpath)
                if not is_soft_link:
                    continue
                link_files[os.path.realpath(filespec.fullpath)] = filespec.fullpath

    for file in link_files:
        interpreter.HandleCommand(
            f"settings append target.source-map {repr(link_files[file])} {repr(os.path.realpath(file))}",
            returnObject,
        )
    interpreter.HandleCommand(f"settings show target.source-map", returnObject)
    print(returnObject.GetOutput())

    # 遍历所有的断点
    for i in range(target.GetNumBreakpoints()):
        breakpoint: lldb.SBBreakpoint = target.GetBreakpointAtIndex(i)
        description = lldb.SBStream()
        breakpoint.GetDescription(description)
        description = description.GetData()
        fields = description.split(", ")
        file_line = [
            field
            for field in fields
            if field.startswith("file = ") or field.startswith("line = ")
        ]
        if len(file_line) != 2:
            continue
        filename = file_line[0][len("file = ") :][1:-1]
        line_number = int(file_line[1][len("line = ") :])
        if filename in link_files:
            target.BreakpointCreateByLocation(filename, line_number)


def __lldb_init_module(debugger: lldb.SBDebugger, internal_dict):
    interpreter: lldb.SBCommandInterpreter = debugger.GetCommandInterpreter()
    returnObject = lldb.SBCommandReturnObject()

    # 设置断点
    interpreter.HandleCommand(
        "br set -n main -o true --auto-continue true", returnObject
    )
    print(returnObject.GetOutput())

    # 在断点触发后继续执行
    interpreter.HandleCommand(
        f"breakpoint command add -s python -o 'cxx_softlink_template_support.callback()'",
        returnObject,
    )
    print(returnObject.GetOutput())


import threading
import lldb
import os
import re
# import lldbsuite.test as lldbtest
# lldbtest.run_suite()


class BreakPointListener:
    def __init__(self, debugger: lldb.SBDebugger):
        self.debugger = debugger
        self.listener = lldb.SBListener("BreakpointListener")
        self.broadcaster = self.debugger.GetSelectedTarget().GetBroadcaster()
        self.setup_breakpoints()
        self.register_listener()

    def setup_breakpoints(self):
        for i in range(self.debugger.GetSelectedTarget().GetNumBreakpoints()):
            breakpoint = self.debugger.GetSelectedTarget().GetBreakpointAtIndex(i)
            print(f"Setup breakpoint {breakpoint}")
            self.map(breakpoint, lldb.eBreakpointEventTypeAdded)

    def get_file_line_by_breakpoint(self, breakpoint: lldb.SBBreakpoint):
        stream = lldb.SBStream()
        breakpoint.GetDescription(stream)
        # "file = '%s', line = %u, "
        match = re.search(
            """file = '(.*)', line = (\d+), """, stream.GetData())
        if match:
            return match.group(1).split("/")[-1], int(match.group(2))
        else:
            return None, None

    def breakpoint_by_name(self, name: str):
        list = lldb.SBBreakpointList(self.debugger.GetSelectedTarget())
        self.debugger.GetSelectedTarget().FindBreakpointsByName(name, list)
        if list.GetSize() > 0:
            return list.GetBreakpointAtIndex(0)
        else:
            return None

    def map(self, breakpoint: lldb.SBBreakpoint, action):
        need_handled = {lldb.eBreakpointEventTypeAdded, lldb.eBreakpointEventTypeRemoved,
                        lldb.eBreakpointEventTypeEnabled, lldb.eBreakpointEventTypeDisabled}
        if action not in need_handled:
            return
        file, line = self.get_file_line_by_breakpoint(breakpoint)
        if file is None or line is None:
            return
        names = lldb.SBStringList()
        breakpoint.GetNames(names)
        if names.GetSize() > 0 and names.GetStringAtIndex(0).startswith("mapping_"):
            return
        name = f"mapping_{file.replace('.','_')}/{line}"
        print(
            f"Mapping {name} to {breakpoint} with action {action} at {file}:{line}")
        if action == lldb.eBreakpointEventTypeAdded:
            point = self.debugger.GetSelectedTarget().BreakpointCreateByLocation(file, line)
            point.AddName(name)
        elif action == lldb.eBreakpointEventTypeRemoved:
            breakpoint = self.breakpoint_by_name(name)
            if breakpoint:
                self.debugger.GetSelectedTarget().BreakpointDelete(breakpoint.GetID())
        elif action == lldb.eBreakpointEventTypeEnabled:
            self.breakpoint_by_name(name).SetEnabled(True)
        elif action == lldb.eBreakpointEventTypeDisabled:
            self.breakpoint_by_name(name).SetEnabled(False)

    def register_listener(self):
        self.broadcaster.AddListener(
            self.listener, lldb.SBTarget.eBroadcastBitBreakpointChanged)
        self.handler = threading.Thread(target=self.listen)
        self.handler.daemon = True
        self.handler.start()

    def listen(self):
        event = lldb.SBEvent()
        try:
            while True:
                if self.listener.WaitForEvent(120, event):
                    if lldb.SBBreakpoint.EventIsBreakpointEvent(event):
                        breakpoint: lldb.SBBreakpoint = lldb.SBBreakpoint.GetBreakpointFromEvent(
                            event)
                        pos: lldb.SBBreakpointLocation = breakpoint.GetLocationAtIndex(
                            0)
                        stream = lldb.SBStream()
                        pos.GetDescription(stream, lldb.eDescriptionLevelFull)
                        addr: lldb.SBAddress = pos.GetAddress()
                        addr.compile_unit.GetFileSpec().SetDirectory(os.getcwd())
                        dir = addr.compile_unit.GetFileSpec().GetDirectory()
                        file = addr.compile_unit.GetFileSpec().GetFilename()
                        line = addr.GetLineEntry().GetLine()
                        # print(stream.GetData())
                        self.map(
                            breakpoint, lldb.SBBreakpoint.GetBreakpointEventTypeFromEvent(event))
        except Exception as e:
            print(e)


class SourceMapping:
    git_repo: str = None

    def __init__(self, debugger: lldb.SBDebugger) -> None:
        self.debugger = debugger

    def find_git_repo(self, path):
        assert path != "/", "Can't find git repo"
        if os.path.exists(os.path.join(path, ".git")):
            return os.path.basename(path)
        return self.find_git_repo(os.path.dirname(path))

    def prefix_folder(self, path: str):
        return path.split(SourceMapping.git_repo)[0]

    def process(self):
        interpreter: lldb.SBCommandInterpreter = self.debugger.GetCommandInterpreter()
        nums_module = self.debugger.GetSelectedTarget().GetNumModules()
        unit_paths = []
        prefix: str = None
        for i in range(nums_module):
            module = self.debugger.GetSelectedTarget().GetModuleAtIndex(i)
            nums_unit = module.GetNumCompileUnits()
            for j in range(nums_unit):
                unit = module.GetCompileUnitAtIndex(j)
                filespec: lldb.SBFileSpec = unit.GetFileSpec()
                unit_paths.append(filespec.fullpath)
                if filespec.basename.startswith("main."):
                    SourceMapping.git_repo = self.find_git_repo(
                        filespec.fullpath)
                    prefix = self.prefix_folder(filespec.fullpath)
        assert prefix is not None, "Can't find main file"
        mapping_array = list()
        for path in map(lambda x: self.prefix_folder(x), filter(lambda x: not os.path.exists(x), unit_paths)):
            mapping_array.append(repr(path))
            mapping_array.append(repr(prefix))
        command = " ".join(mapping_array)
        returnObject = lldb.SBCommandReturnObject()
        interpreter.HandleCommand(
            f"settings set target.source-map {command}", returnObject)
        assert returnObject.Succeeded(), returnObject.GetError()
        interpreter.HandleCommand(
            f"settings show target.source-map", returnObject)
        print(returnObject.GetOutput())


listener: lldb.SBListener = None


def __lldb_init_module(debugger, internal_dict):
    print("init module")
    host = "127.0.0.1"  # or "localhost"
    port = 12340
    print("Waiting for debugger attach at %s:%s ......" % (host, port))
    import ptvsd
    ptvsd.enable_attach(address=(host, port), redirect_output=True)
    ptvsd.wait_for_attach()
    ptvsd.break_into_debugger()
    SourceMapping(debugger).process()

    # global listener
    # listener = BreakPointListener(debugger)
    # debugger.HandleCommand('command script add -f main.your_first_command cat -h "main command ----12312312"')

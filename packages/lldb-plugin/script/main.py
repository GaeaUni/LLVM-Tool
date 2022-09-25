import pdb
import sys
# import lldb
import ptvsd



def your_first_command(debugger, command, result, internal_dict):
    print("your_first_command")
    host = "127.0.0.1" # remote host ip
    port = 12345    # remote host valid port
    ptvsd.enable_attach(address=(host, port), redirect_output=True)
    ptvsd.wait_for_attach()
    # import pydevd_pycharm
    # print("sys.path: %s" % sys.path)
    # print("0000")
    # pydevd_pycharm.settrace('localhost', port=12345, stdoutToServer=True, stderrToServer=True)
    # print("1111")
    # print("your_first_command!")
    # debugger = lldb.SBDebugger.Create()
    # debugger.SetAsync(False)
    # target = debugger.CreateTargetWithFileAndArch(None, lldb.LLDB_ARCH_DEFAULT)
    # target.BreakpointCreateByName("main", "a.out")
    # process = target.LaunchSimple(None, None, "/home/developer")
    # print("process: %s" % process)
    # print("process.GetState(): %s" % process.GetState())
    # print("process.GetExitStatus(): %s" % process.GetExitStatus())
    # print("process.GetExitDescription(): %s" % process.GetExitDescription())
    # print("process.GetNumThreads(): %s" % process.GetNumThreads())
    # print("process.GetThreadAtIndex(0): %s" % process.GetThreadAtIndex(0))
    # print("process.GetThreadAtIndex(0).GetNumFrames(): %s" % process.GetThreadAtIndex(0).GetNumFrames())
    # print("process.GetThreadAtIndex(0).GetFrameAtIndex(0): %s" % process.GetThreadAtIndex(0).GetFrameAtIndex(0))
    # print("process.GetThreadAtIndex(0).GetFrameAtIndex(0).GetLineEntry(): %s" % process.GetThreadAtIndex(0).GetFrameAtIndex(0).GetLineEntry())
    print("Done")


def __lldb_init_module(debugger, internal_dict):
    print("init module")
    debugger.HandleCommand('command script add -f main.your_first_command cat -h "main command ----12312312"')

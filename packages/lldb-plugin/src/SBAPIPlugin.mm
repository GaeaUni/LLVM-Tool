//
//  SBAPIPlugin.m
//  SBAPIPlugin
//
//  Created by 周亮 on 2022/9/5.
//

#import "SBAPIPlugin.h"
#include <string>
#include <iostream>

namespace kk {
bool FindAddressCommand::DoExecute(lldb::SBDebugger debugger, char **command, lldb::SBCommandReturnObject &result) {
    std::cout << "FindAddressCommand::DoExecute" << std::endl;
    auto addr = std::stoul(command[0]);
    printf("%x",addr);
    // 1.扫描mach-o

    // 2.扫描stack
    // 3.扫描heap

    return true;
}
}

namespace lldb {
bool PluginInitialize(SBDebugger debugger)
{
    lldb::SBCommandInterpreter interpreter = debugger.GetCommandInterpreter();
    lldb::SBCommand command = interpreter.AddMultiwordCommand("kk", "kk command");
    command.AddCommand("find", new kk::FindAddressCommand("find"), "find address");
    return true;
}
}

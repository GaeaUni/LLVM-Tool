//
//  SBAPIPlugin.m
//  SBAPIPlugin
//
//  Created by 周亮 on 2022/9/5.
//

#import "SBAPIPlugin.h"
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include "LLDB/SBAddress.h"
#include "LLDB/SBTarget.h"

namespace kk {
bool isStackAddress(lldb::SBAddress address, lldb::SBTarget target) {
    auto thread = target.GetProcess().GetSelectedThread();
    auto frameCount = thread.GetNumFrames();
    for (int i = 0; i < frameCount; i++) {
        auto frame = thread.GetFrameAtIndex(i);
        auto top = frame.GetFP();
        auto bottom = frame.GetSP();
        if (address.GetLoadAddress(target) >= bottom && address.GetLoadAddress(target) <= top) {
            return true;
        }
    }
    return false;
}

bool FindAddressCommand::DoExecute(lldb::SBDebugger debugger, char **command, lldb::SBCommandReturnObject &result) {
    std::cout << "FindAddressCommand::DoExecute" << std::endl;
    auto target = debugger.GetSelectedTarget();
    auto thread = target.GetProcess().GetSelectedThread();
    if (!thread.IsValid()) {
        result.SetError("No thread selected");
        return false;
    }
    std::string lower = command[0];
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    auto addr = std::stoul(lower, nullptr, 16);
    std::cout << "addr = " << addr << std::endl;
    auto sbAddr = target.ResolveLoadAddress(addr);
    // 1.扫描mach-o
    auto section = sbAddr.GetSection();
    if (section) {
        std::cout << "section = " << section.GetName() << std::endl;
        auto parent = section.GetParent();
        if (parent) {
            std::cout << "parent = " << parent.GetName() << std::endl;
        }
        auto module = sbAddr.GetModule();
        if (module) {
            std::cout << "module = " << module.GetFileSpec().GetFilename() << std::endl;
        }
        auto addrOffset = sbAddr.GetFileAddress() - section.GetFileAddress();
        addrOffset = sbAddr.GetLoadAddress(target) - section.GetLoadAddress(target);
        addrOffset = sbAddr.GetOffset();
        std::cout << "addrOffset = " << addrOffset << std::endl;

        auto symbol = sbAddr.GetSymbol();
        if (symbol) {
            std::cout << "symbol = " << symbol.GetName() << std::endl;
        }
        auto symbolOffset = sbAddr.GetLoadAddress(target) - symbol.GetStartAddress().GetLoadAddress(target);
        std::cout << "symbolOffset = " << symbolOffset << std::endl;

        auto mangled = symbol.GetMangledName();
        if (mangled) {
            std::cout << "mangled = " << mangled << std::endl;
        }
        auto isExternal = symbol.IsExternal();
        std::cout << "isExternal = " << isExternal << std::endl;
    }
    // 2.扫描stack
    if (isStackAddress(sbAddr, target)) {
        std::cout << "stack" << std::endl;
    }
    // 3.扫描heap
    return true;
}
}  // namespace kk

namespace lldb {
bool PluginInitialize(SBDebugger debugger) {
    lldb::SBCommandInterpreter interpreter = debugger.GetCommandInterpreter();
    lldb::SBCommand command = interpreter.AddMultiwordCommand("kk", "kk command");
    command.AddCommand("find", new kk::FindAddressCommand("find"), "find address");
    return true;
}
}

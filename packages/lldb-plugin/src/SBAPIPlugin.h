//
//  SBAPIPlugin.h
//  SBAPIPlugin
//
//  Created by 周亮 on 2022/9/5.
//

#import <LLDB/lldb.h>

#import <string>
#include <utility>

namespace kk {
class FindAddressCommand : public lldb::SBCommandPluginInterface {
 public:
    FindAddressCommand(std::string name) : name(std::move(name)) {
    }
    bool DoExecute(lldb::SBDebugger /*debugger*/, char** /*command*/, lldb::SBCommandReturnObject& /*result*/) override;

 private:
    std::string name;
};
}  // namespace kk

namespace lldb {
bool PluginInitialize(SBDebugger debugger);
}

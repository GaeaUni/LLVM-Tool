#include "CallStorage.h"

#include <fstream>

#include "ImplementProtocolStorage.h"
#include "Utils.h"
#include "llvm/Support/JSON.h"
using namespace llvm;
using namespace llvm::json;

static vector<string> Protocol2Implements(string protocol) {
    if (protocol.find("<") != string::npos && protocol.find(">") != string::npos) {
        protocol = protocol.substr(protocol.find("<") + 1, protocol.find(">") - protocol.find("<") - 1);
    } else {
        return {};
    }
    auto &storage = ImplementProtocolStorage::shared();
    auto implements = storage.getImplementsFromProtocol(protocol);
    return implements;
}

bool isProtocolCall(const ObjCMessageExpr *call) {
    if (call->getReceiverInterface()) {
        return false;
    } else {
        return true;
    }
}

void CallStorage::addOneCall(const ObjCImplementationDecl *objcClass, const ObjCMethodDecl *method,
                             const ObjCMessageExpr *call) {
    auto className = CallGraph::Parse(objcClass);
    auto methodName = CallGraph::Parse(method);
    auto callExpr = CallGraph::Parse(call);
    callMap[className][methodName].emplace_back(callExpr);

    if (isProtocolCall(call)) {
        auto implements = Protocol2Implements(callExpr.first);
        for (auto implement : implements) {
            auto methodFullName = implement + "/" + callExpr.second;
            method2Callers[methodFullName].emplace_back(className + "/" + methodName);
        }
    } else {
        auto methodFullName = callExpr.first + "/" + callExpr.second;
        method2Callers[methodFullName].emplace_back(className + "/" + methodName);
        if (callExpr.second == "+new") {
            // +new = alloc_init
            methodFullName = callExpr.first + "/" + "-init";
            method2Callers[methodFullName].emplace_back(className + "/" + methodName);
        }
    }
}

void CallStorage::printAll() {
    //        for(auto &class2Method2Calls : callMap) {
    //            cout << "类 ： " << class2Method2Calls.first << endl;
    //            for(auto &method2Calls : class2Method2Calls.second) {
    //                cout << " <->方法 : " << method2Calls.first << endl;
    //                for (auto &call : method2Calls.second) {
    //                    cout << "    " << call.first << " " << call.second << endl;
    //                }
    //            }
    //        }
    //        for (auto &mc : method2Callers) {
    //            cout << "方法" << mc.first << " 调用者：" << endl;
    //            for (auto caller : mc.second) {
    //                cout << caller << endl;
    //            }
    //        }
    //        cout << toJson() << endl;
}

string CallStorage::dumpMethod2CallersJson(const string &filename) {
    std::string S;
    llvm::raw_string_ostream OS(S);
    OStream J(OS, 2);
    J.object([&] {
        for (auto &mc : method2Callers) {
            J.attributeArray(mc.first, [&] {
                for (auto caller : mc.second) {
                    J.value(caller);
                }
            });
        }
    });
    std::ofstream out(filename);
    out << S;
    out.close();
    return S;
}
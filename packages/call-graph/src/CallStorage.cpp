#include "CallStorage.h"

#include <fstream>

#include "ImplementProtocolStorage.h"
#include "Utils.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/JSON.h"
using namespace llvm;
using namespace llvm::json;

static vector<string> Protocol2Implements(const ObjCMessageExpr *call) {
    auto &storage = ImplementProtocolStorage::shared();
    auto idType = call->getReceiverType()->getAsObjCQualifiedIdType();
    auto count = idType->getNumProtocols();
    vector<string> result;
    for (size_t i = 0; i < count; i++) {
        auto protocol = idType->getProtocol(i);
        auto implements = storage.getImplementsFromProtocol(protocol->getNameAsString());
        result.insert(result.end(), implements.begin(), implements.end());
    }
    return result;
}

bool IsProtocolCall(const ObjCMessageExpr *call) {
    if (call->getReceiverInterface()) {
        return false;
    } else {
        return true;
    }
}

bool IsEqual(string s1Name, string s2Name) {
    if (s1Name == "new") {
        s1Name = "init";
    }
    if (s2Name == "new") {
        s2Name = "init";
    }
    return s1Name == s2Name;
}

bool IsEqual(const Selector &s1, const Selector &s2) {
    return IsEqual(s1.getAsString(), s2.getAsString());
}

bool IsSystemCall(pair<ObjCInterfaceDecl *, ObjCProtocolDecl *> call, const SourceManager *manager) {
    auto sourceFile = manager->getFilename(call.first->getSourceRange().getBegin());
    if (!sourceFile.startswith("/Applications/Xcode.app")) {
        return false;
    }
    if (call.second) {
        auto sourceFile = manager->getFilename(call.second->getSourceRange().getBegin());
        if (!sourceFile.startswith("/Applications/Xcode.app")) {
            return false;
        }
    }
    return true;
}

pair<ObjCInterfaceDecl *, ObjCProtocolDecl *> getRealReceiver(const ObjCMessageExpr *call) {
    auto selector = call->getSelector();
    for (auto receiver = call->getReceiverInterface(); receiver; receiver = receiver->getSuperClass()) {
        auto implementMethods = ImplementProtocolStorage::shared().getMethodsFromImplement(receiver->getNameAsString());
        if (!implementMethods.empty()) {
            for (auto method : implementMethods) {
                if (IsEqual(method, selector.getAsString())) {
                    return {receiver, nullptr};
                }
            }
        } else {
            auto methods = receiver->methods();
            for (auto method : methods) {
                if (IsEqual(method->getSelector(), selector)) {
                    return {receiver, nullptr};
                }
            }
        }
        auto protocols = receiver->protocols();
        for (auto &protocol : protocols) {
            auto methods = protocol->methods();
            for (auto method : methods) {
                if (IsEqual(method->getSelector(), selector)) {
                    return {receiver, protocol};
                }
            }
        }
    }
    return {nullptr, nullptr};
}

void CallStorage::addOneCall(const ObjCImplementationDecl *objcClass, const ObjCMethodDecl *method,
                             const ObjCMessageExpr *call, const SourceManager *manager) {
    auto className = CallGraph::Parse(objcClass);
    auto methodName = CallGraph::Parse(method);
    auto callExpr = CallGraph::Parse(call);
    callMap[className][methodName].emplace_back(callExpr);

    if (IsProtocolCall(call)) {
        auto implements = Protocol2Implements(call);
        for (auto implement : implements) {
            auto methodFullName = implement + "/" + callExpr.second;
            method2Callers[methodFullName].emplace_back(className + "/" + methodName);
        }
    } else {
        auto caller = getRealReceiver(call);
        if (!caller.first && !caller.second) {
            return;
        }
        if (IsSystemCall(caller, manager)) {
            return;
        }
        auto methodFullName = caller.first->getNameAsString() + "/" + callExpr.second;
        method2Callers[methodFullName].emplace_back(className + "/" + methodName);
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
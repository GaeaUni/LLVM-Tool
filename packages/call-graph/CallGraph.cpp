#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/Program.h"

using namespace clang::tooling;
using namespace clang;
using namespace std;
using namespace llvm;
using namespace clang::ast_matchers;
using namespace json;

namespace CallGraph {

constexpr char objcClass[] = "objcClass";
constexpr char caller[] = "caller";
constexpr char selector[] = "selector";

class DiffStorage {
 public:
    static DiffStorage &shared() {
        static DiffStorage sharedInstance;
        return sharedInstance;
    }

    vector<pair<int, int>> getFileDiff(const string &filename) {
        return file2Diffs[filename];
    }

    void parse(const string &filename) {
        std::ifstream diffFile(filename);
        std::stringstream ss;
        ss << diffFile.rdbuf();
        string content = ss.str();
        Expected<Value> diffJson = json::parse(content);
        auto root = diffJson->getAsObject();
        for (auto i = root->begin(); i != root->end(); i++) {
            auto file = i->getFirst().str();
            auto diffs = i->getSecond().getAsArray();
            for (auto diff = diffs->begin(); diff != diffs->end(); diff++) {
                int start = diff->getAsObject()->getNumber("startLine").getValue();
                int end = diff->getAsObject()->getNumber("endLine").getValue();
                auto pair = make_pair(start, end);
                file2Diffs[file].emplace_back(pair);
            }
        }
    }

    void printAll() {
        for (auto &kv : class2DiffMethods) {
            cout << "类：" << kv.first << "被修改过的方法：" << endl;
            for (auto &method : kv.second) {
                cout << "  " << method << endl;
            }
        }
    }

    void appendDiffMethod(const string &className, const string &method) {
        class2DiffMethods[className].emplace_back(method);
    }

    string dumpClass2DiffsJson(const string &filename) {
        std::string S;
        llvm::raw_string_ostream OS(S);
        OStream J(OS, 2);
        J.array([&] {
            for (auto &diffs : class2DiffMethods) {
                for (auto method : diffs.second) {
                    J.value(diffs.first + "/" + method);
                }
            }
        });
        std::ofstream out(filename);
        out << S;
        out.close();
        return S;
    }

 private:
    map<string, vector<pair<int, int>>> file2Diffs;
    map<string, vector<string>> class2DiffMethods;
    DiffStorage(){};
    ~DiffStorage(){};
    DiffStorage(const DiffStorage &) = delete;
    DiffStorage(const DiffStorage &&) = delete;
    DiffStorage &operator=(const DiffStorage &) = delete;
};

string selector2String(const Selector &selector) {
    auto s = selector.getAsString();
    for (auto i = s.begin(); i != s.end(); i++) {
        if (*i == ':') {
            *i = '_';
        }
    }
    return s;
}

string parse(const ObjCImplementationDecl *objcClass) {
    return objcClass->getIdentifier()->getName().str();
}

string parse(const ObjCMethodDecl *method) {
    return (method->isClassMethod() ? "+" : "-") + selector2String(method->getSelector());
}

pair<string, string> parse(const ObjCMessageExpr *call) {
    string receiver;
    if (call->isClassMessage()) {
        receiver = call->getClassReceiver().getAsString();
    } else if (call->isInstanceMessage()) {
        //        if (auto instance = call->getInstanceReceiver()) {
        //            receiver = instance->getType().getAsString();
        //        } else {
        //            // 好像有点坑，暂时没想到什么其它情况能走到这
        //            receiver = call->getSuperType().getAsString();
        //        }
        receiver = call->getReceiverType().getAsString();
        receiver = receiver.substr(0, receiver.find_first_of(' '));
    } else {
        assert(false);
    }
    auto selector = (call->isClassMessage() ? "+" : "-") + selector2String(call->getSelector());
    return make_pair(receiver, selector);
}

class MethodDiffQueryCallback : public MatchFinder::MatchCallback {
    /*
     找出哪些类中哪些方法被修改过
    */
 public:
    void run(const MatchFinder::MatchResult &Result) override {
        auto method = Result.Nodes.getNodeAs<ObjCMethodDecl>(selector);
        assert(Result.Nodes.getMap().size() == 2);
        auto methodClass = Result.Nodes.getNodeAs<ObjCImplementationDecl>(objcClass);
        ;
        auto isDiff = isInDiffMethod(method, Result.SourceManager);

        if (isDiff) {
            DiffStorage &diffParser = DiffStorage::shared();
            diffParser.appendDiffMethod(parse(methodClass), parse(method));
        }
    }

 private:
    bool isInDiffMethod(const ObjCMethodDecl *method, const SourceManager *sourceManager) {
        auto start = method->getSourceRange().getBegin();
        auto end = method->getSourceRange().getEnd();

        auto filename = sourceManager->getFilename(start).str();
        auto startLineNumber = sourceManager->getPresumedLineNumber(start);
        auto endLineNumber = sourceManager->getPresumedLineNumber(end);

        DiffStorage &diffParser = DiffStorage::shared();
        auto diffs = diffParser.getFileDiff(filename);

        auto result = find_if(diffs.begin(), diffs.end(), [=](pair<unsigned int, unsigned int> diff) -> bool {
            auto condition1 = startLineNumber >= diff.first && startLineNumber <= diff.second;
            auto condition2 = endLineNumber >= diff.first && endLineNumber <= diff.second;
            auto condition3 = startLineNumber <= diff.first && endLineNumber >= diff.second;
            return condition1 || condition2 || condition3;
        });
        return result != diffs.end();
    }
};

class CallStorage {
 public:
    void addOneCall(const ObjCImplementationDecl *objcClass, const ObjCMethodDecl *method,
                    const ObjCMessageExpr *call) {
        auto className = parse(objcClass);
        auto methodName = parse(method);
        auto callExpr = parse(call);
        callMap[className][methodName].emplace_back(callExpr);

        auto methodFullName = callExpr.first + "/" + callExpr.second;
        method2Callers[methodFullName].emplace_back(className + "/" + methodName);
        if (callExpr.second == "+new") {
            // +new = alloc_init
            methodFullName = callExpr.first + "/" + "-init";
            method2Callers[methodFullName].emplace_back(className + "/" + methodName);
        }

        // TODO: 分析protocol的调用方式
        auto ps = call->getReceiverInterface()->protocols();
        for (auto &p : ps) {
            auto protocolName = p->getName().str();

            cout << protocolName;
        }
    }

    void printAll() {
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

    string dumpMethod2CallersJson(const string &filename) {
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

    static CallStorage &shared() {
        static CallStorage sharedInstance;
        return sharedInstance;
    }

 private:
    CallStorage(){};
    ~CallStorage(){};
    CallStorage(const CallStorage &) = delete;
    CallStorage(const CallStorage &&) = delete;
    CallStorage &operator=(const CallStorage &) = delete;

 private:
    // 类名: (方法名 ： [receiver, selector])
    using OneCall = vector<pair<string, string>>;
    using Method2Calls = std::map<string, OneCall>;
    std::map<string, Method2Calls> callMap;

    std::map<string, vector<string>> method2Callers;
};

class CallGraphPropertyCallback : public MatchFinder::MatchCallback {
 public:
    void run(const MatchFinder::MatchResult &Result) override {
        assert(Result.Nodes.getMap().size() == 3);
        auto method = Result.Nodes.getNodeAs<ObjCMethodDecl>(selector);
        auto methodClass = Result.Nodes.getNodeAs<ObjCImplementationDecl>(objcClass);
        auto oneCall = Result.Nodes.getNodeAs<ObjCMessageExpr>(caller);
        CallStorage::shared().addOneCall(methodClass, method, oneCall);
    }
};
}  // namespace CallGraph

using namespace CallGraph;

int main(int argc, const char **argv) {
    static cl::OptionCategory CallGraphToolCategory("CallGraphToolCategory options");
    auto OptionsParser = CommonOptionsParser::create(argc, argv, CallGraphToolCategory);
    auto diffPath = OptionsParser.get().getSourcePathList().front();

    DiffStorage::shared().parse(diffPath);
    auto files = OptionsParser.get().getCompilations().getAllFiles();
    ClangTool Tool(OptionsParser.get().getCompilations(), files);

    auto matcherMethodDecl = objcMethodDecl(hasAncestor(objcImplementationDecl().bind(objcClass))).bind(selector);

    auto matcherCallDecl = objcMethodDecl(hasAncestor(objcImplementationDecl().bind(objcClass)),
                                          forEachDescendant(objcMessageExpr().bind(caller)))
                               .bind(selector);

    MatchFinder Finder;
    CallGraphPropertyCallback callGraphPropertyCallback;
    MethodDiffQueryCallback methodDiffQueryCallback;
    Finder.addMatcher(matcherMethodDecl, &methodDiffQueryCallback);
    Finder.addMatcher(matcherCallDecl, &callGraphPropertyCallback);
    auto statusCode = Tool.run(newFrontendActionFactory(&Finder).get());
    CallStorage::shared().printAll();
    DiffStorage::shared().printAll();

    auto pyWorkPath = diffPath.substr(0, diffPath.find_last_of('/'));
    CallStorage::shared().dumpMethod2CallersJson(pyWorkPath + "/call_map.json");
    DiffStorage::shared().dumpClass2DiffsJson(pyWorkPath + "/init_nodes.json");
    return statusCode;
}

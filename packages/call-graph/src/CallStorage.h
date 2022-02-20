
#include <map>
#include <string>
#pragma once
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ExprObjC.h"

using namespace clang;
using namespace std;

class CallStorage {
 public:
    void addOneCall(const ObjCImplementationDecl *objcClass, const ObjCMethodDecl *method, const ObjCMessageExpr *call);

    void printAll();

    string dumpMethod2CallersJson(const string &filename);

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
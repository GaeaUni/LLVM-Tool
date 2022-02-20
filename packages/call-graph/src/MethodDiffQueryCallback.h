#include "DiffStorage.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#pragma once
using namespace clang;
using namespace clang::ast_matchers;

class MethodDiffQueryCallback {
    /*
     找出哪些类中哪些方法被修改过
    */
 public:
    void run(const MatchFinder::MatchResult &Result, const ObjCMethodDecl *method,
             const ObjCImplementationDecl *methodClass);

 private:
    bool isInDiffMethod(const ObjCMethodDecl *method, const SourceManager *sourceManager);
};
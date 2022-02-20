#include "ImplementProtocolCallback.h"
#include "MethodDiffQueryCallback.h"
#include "clang/AST/DeclObjC.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#pragma once
using namespace clang::ast_matchers;
using namespace clang;
class ImplementCallback : public MatchFinder::MatchCallback {
 public:
    virtual void run(const MatchFinder::MatchResult &Result);

 private:
    MethodDiffQueryCallback diffCallback;
    ImplementProtocolCallback protocolCallback;
};
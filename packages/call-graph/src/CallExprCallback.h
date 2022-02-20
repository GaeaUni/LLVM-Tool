#include "clang/ASTMatchers/ASTMatchFinder.h"
#pragma once
using namespace clang::ast_matchers;
using namespace clang;

class CallExprCallback : public MatchFinder::MatchCallback {
 public:
    void run(const MatchFinder::MatchResult &Result) override;

 private:
    bool isSystemCall(const SourceManager *manager, const ObjCMessageExpr *call);
};
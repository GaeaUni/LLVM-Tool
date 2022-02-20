#include <string>
#include <vector>
#pragma once
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ExprObjC.h"

namespace CallGraph {
using namespace std;
using namespace clang;
constexpr char objcClass[] = "objcClass";
constexpr char caller[] = "caller";
constexpr char selector[] = "selector";

string Selector2String(const Selector &selector);

string Parse(const ObjCImplementationDecl *objcClass);

string Parse(const ObjCMethodDecl *method);

pair<string, string> Parse(const ObjCMessageExpr *call);
}  // namespace CallGraph
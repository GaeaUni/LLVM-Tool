#include "clang/AST/DeclObjC.h"
#pragma once
using namespace clang;

class ImplementProtocolCallback {
 public:
    void run(const ObjCImplementationDecl *objcClass);
};
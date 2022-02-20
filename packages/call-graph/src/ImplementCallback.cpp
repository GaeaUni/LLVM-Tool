#include "ImplementCallback.h"

#include "Utils.h"

void ImplementCallback::run(const MatchFinder::MatchResult &Result) {
    assert(Result.Nodes.getMap().size() == 1);
    auto methodClass = Result.Nodes.getNodeAs<ObjCImplementationDecl>(CallGraph::objcClass);
    auto methods = methodClass->methods();
    for (auto method : methods) {
        diffCallback.run(Result, method, methodClass);
    }
    protocolCallback.run(methodClass);
}
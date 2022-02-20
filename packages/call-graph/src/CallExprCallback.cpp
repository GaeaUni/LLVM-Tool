#include "CallExprCallback.h"

#include "CallStorage.h"
#include "Utils.h"
void CallExprCallback::run(const MatchFinder::MatchResult &Result) {
    assert(Result.Nodes.getMap().size() == 3);
    auto oneCall = Result.Nodes.getNodeAs<ObjCMessageExpr>(CallGraph::caller);
    if (isSystemCall(Result.SourceManager, oneCall)) {
        return;
    }
    auto method = Result.Nodes.getNodeAs<ObjCMethodDecl>(CallGraph::selector);
    auto methodClass = Result.Nodes.getNodeAs<ObjCImplementationDecl>(CallGraph::objcClass);
    CallStorage::shared().addOneCall(methodClass, method, oneCall);
}

bool CallExprCallback::isSystemCall(const SourceManager *manager, const ObjCMessageExpr *call) {
    auto interface = call->getReceiverInterface();
    if (!interface) {
        return false;
    }
    auto sourceFile = manager->getFilename(interface->getSourceRange().getBegin());
    if (sourceFile.startswith("/Applications/Xcode.app")) {
        return true;
    } else {
        return false;
    }
}
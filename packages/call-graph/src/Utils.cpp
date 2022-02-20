#include "Utils.h"

namespace CallGraph {
string Selector2String(const Selector &selector) {
    auto s = selector.getAsString();
    for (auto i = s.begin(); i != s.end(); i++) {
        if (*i == ':') {
            *i = '_';
        }
    }
    return s;
}

string Parse(const ObjCImplementationDecl *objcClass) {
    return objcClass->getIdentifier()->getName().str();
}

string Parse(const ObjCMethodDecl *method) {
    return (method->isClassMethod() ? "+" : "-") + Selector2String(method->getSelector());
}

pair<string, string> Parse(const ObjCMessageExpr *call) {
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
    auto selector = (call->isClassMessage() ? "+" : "-") + Selector2String(call->getSelector());
    return make_pair(receiver, selector);
}
}  // namespace CallGraph

#include "ImplementProtocolCallback.h"

#include "ImplementProtocolStorage.h"
#include "Utils.h"
void ImplementProtocolCallback::run(const ObjCImplementationDecl *objcClass) {
    auto name = CallGraph::Parse(objcClass);
    for (auto protocol : objcClass->getClassInterface()->protocols()) {
        ImplementProtocolStorage::shared().addProtocol(name, protocol->getNameAsString());
    }
}
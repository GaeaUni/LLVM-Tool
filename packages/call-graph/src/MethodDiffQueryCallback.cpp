#include "MethodDiffQueryCallback.h"

#include "Utils.h"

void MethodDiffQueryCallback::run(const MatchFinder::MatchResult &Result, const ObjCMethodDecl *method,
                                  const ObjCImplementationDecl *methodClass) {
    auto isDiff = isInDiffMethod(method, Result.SourceManager);
    if (isDiff) {
        DiffStorage &diffParser = DiffStorage::shared();
        diffParser.appendDiffMethod(CallGraph::Parse(methodClass), CallGraph::Parse(method));
    }
}

bool MethodDiffQueryCallback::isInDiffMethod(const ObjCMethodDecl *method, const SourceManager *sourceManager) {
    auto start = method->getSourceRange().getBegin();
    auto end = method->getSourceRange().getEnd();

    auto filename = sourceManager->getFilename(start).str();
    auto startLineNumber = sourceManager->getPresumedLineNumber(start);
    auto endLineNumber = sourceManager->getPresumedLineNumber(end);

    DiffStorage &diffParser = DiffStorage::shared();
    auto diffs = diffParser.getFileDiff(filename);

    auto result = find_if(diffs.begin(), diffs.end(), [=](pair<unsigned int, unsigned int> diff) -> bool {
        auto condition1 = startLineNumber >= diff.first && startLineNumber <= diff.second;
        auto condition2 = endLineNumber >= diff.first && endLineNumber <= diff.second;
        auto condition3 = startLineNumber <= diff.first && endLineNumber >= diff.second;
        return condition1 || condition2 || condition3;
    });
    return result != diffs.end();
}
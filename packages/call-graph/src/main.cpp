#include "CallExprCallback.h"
#include "CallStorage.h"
#include "DiffStorage.h"
#include "ImplementCallback.h"
#include "Utils.h"
#include "clang/AST/AST.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
using namespace llvm;
using namespace clang;
using namespace clang::tooling;

int main(int argc, const char **argv) {
    static cl::OptionCategory CallGraphToolCategory("CallGraphToolCategory options");
    auto OptionsParser = CommonOptionsParser::create(argc, argv, CallGraphToolCategory);
    if (!OptionsParser) {
        llvm::errs() << OptionsParser.takeError();
        return 1;
    }
    CommonOptionsParser &OP = OptionsParser.get();
    auto diffPath = OP.getSourcePathList().front();

    DiffStorage::shared().parse(diffPath);
    auto files = OP.getCompilations().getAllFiles();
    ClangTool Tool(OP.getCompilations(), files);

    auto matcherImplementDecl = objcImplementationDecl(isExpansionInMainFile()).bind(CallGraph::objcClass);

    auto matcherCallDecl =
        objcMethodDecl(hasAncestor(objcImplementationDecl(isExpansionInMainFile()).bind(CallGraph::objcClass)),
                       forEachDescendant(objcMessageExpr().bind(CallGraph::caller)))
            .bind(CallGraph::selector);

    MatchFinder Finder;
    CallExprCallback callGraphPropertyCallback;
    ImplementCallback implementCallback;
    Finder.addMatcher(matcherImplementDecl, &implementCallback);
    Finder.addMatcher(matcherCallDecl, &callGraphPropertyCallback);
    auto statusCode = Tool.run(newFrontendActionFactory(&Finder).get());
    CallStorage::shared().printAll();
    DiffStorage::shared().printAll();

    auto pyWorkPath = diffPath.substr(0, diffPath.find_last_of('/'));
    CallStorage::shared().dumpMethod2CallersJson(pyWorkPath + "/call_map.json");
    DiffStorage::shared().dumpClass2DiffsJson(pyWorkPath + "/init_nodes.json");
    return statusCode;
}

#include "DiffStorage.h"

#include "llvm/Support/JSON.h"
using namespace llvm::json;
using namespace llvm;

vector<pair<int, int>> DiffStorage::getFileDiff(const string &filename) {
    return file2Diffs[filename];
}

void DiffStorage::parse(const string &filename) {
    std::ifstream diffFile(filename);
    std::stringstream ss;
    ss << diffFile.rdbuf();
    string content = ss.str();
    Expected<Value> diffJson = json::parse(content);
    if (!diffJson) {
        llvm::errs() << "Failed to parse diff file: " << diffJson.takeError() << "\n";
        return;
    }
    auto root = diffJson->getAsObject();

    for (auto i = root->begin(); i != root->end(); i++) {
        auto file = i->getFirst().str();
        auto diffs = i->getSecond().getAsArray();
        for (auto diff = diffs->begin(); diff != diffs->end(); diff++) {
            int start = diff->getAsObject()->getNumber("startLine").getValue();
            int end = diff->getAsObject()->getNumber("endLine").getValue();
            auto pair = make_pair(start, end);
            file2Diffs[file].emplace_back(pair);
        }
    }
}

void DiffStorage::printAll() {
    for (auto &kv : class2DiffMethods) {
        cout << "类：" << kv.first << "被修改过的方法：" << endl;
        for (auto &method : kv.second) {
            cout << "  " << method << endl;
        }
    }
}

void DiffStorage::appendDiffMethod(const string &className, const string &method) {
    class2DiffMethods[className].emplace_back(method);
}

string DiffStorage::dumpClass2DiffsJson(const string &filename) {
    std::string S;
    llvm::raw_string_ostream OS(S);
    OStream J(OS, 2);
    J.array([&] {
        for (auto &diffs : class2DiffMethods) {
            for (auto method : diffs.second) {
                J.value(diffs.first + "/" + method);
            }
        }
    });
    std::ofstream out(filename);
    out << S;
    out.close();
    return S;
}
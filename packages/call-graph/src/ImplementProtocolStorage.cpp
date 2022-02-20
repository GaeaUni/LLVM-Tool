#include "ImplementProtocolStorage.h"

void ImplementProtocolStorage::addProtocol(string implement, string protocol) {
    implement2Protocols[implement].insert(protocol);
}

void ImplementProtocolStorage::dump() {
    for (auto &kv : implement2Protocols) {
        cout << "类：" << kv.first << "实现的协议：" << endl;
        for (auto &protocol : kv.second) {
            cout << "  " << protocol << endl;
        }
    }
}

vector<string> ImplementProtocolStorage::getImplementsFromProtocol(string protocol) {
    vector<string> result;
    for (auto &kv : implement2Protocols) {
        if (kv.second.find(protocol) != kv.second.end()) {
            result.push_back(kv.first);
        }
    }
    return result;
}
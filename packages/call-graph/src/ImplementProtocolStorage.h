#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#pragma once
using namespace std;

class ImplementProtocolStorage {
 public:
    static ImplementProtocolStorage &shared() {
        static ImplementProtocolStorage sharedInstance;
        return sharedInstance;
    }

    void addProtocol(string implement, string protocol);

    vector<string> getImplementsFromProtocol(string protocol);

    void dump();

 private:
    map<string, set<string>> implement2Protocols;
    ImplementProtocolStorage(){};
    ~ImplementProtocolStorage(){};
    ImplementProtocolStorage(const ImplementProtocolStorage &) = delete;
    ImplementProtocolStorage(const ImplementProtocolStorage &&) = delete;
    ImplementProtocolStorage &operator=(const ImplementProtocolStorage &) = delete;
};
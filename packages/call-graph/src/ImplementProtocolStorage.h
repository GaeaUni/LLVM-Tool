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

    void addMethods(string implement, vector<string> methods);

    vector<string> getImplementsFromProtocol(string protocol);

    vector<string> getMethodsFromImplement(string implement);

    void dump();

 private:
    map<string, set<string>> implement2Protocols;
    map<string, set<string>> implement2Methods;
    ImplementProtocolStorage(){};
    ~ImplementProtocolStorage(){};
    ImplementProtocolStorage(const ImplementProtocolStorage &) = delete;
    ImplementProtocolStorage(const ImplementProtocolStorage &&) = delete;
    ImplementProtocolStorage &operator=(const ImplementProtocolStorage &) = delete;
};
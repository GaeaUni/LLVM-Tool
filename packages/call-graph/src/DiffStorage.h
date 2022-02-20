
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#pragma once
using namespace std;

class DiffStorage {
 public:
    static DiffStorage &shared() {
        static DiffStorage sharedInstance;
        return sharedInstance;
    }

    vector<pair<int, int>> getFileDiff(const string &filename);

    void parse(const string &filename);

    void printAll();

    void appendDiffMethod(const string &className, const string &method);

    string dumpClass2DiffsJson(const string &filename);

 private:
    map<string, vector<pair<int, int>>> file2Diffs;
    map<string, vector<string>> class2DiffMethods;
    DiffStorage(){};
    ~DiffStorage(){};
    DiffStorage(const DiffStorage &) = delete;
    DiffStorage(const DiffStorage &&) = delete;
    DiffStorage &operator=(const DiffStorage &) = delete;
};

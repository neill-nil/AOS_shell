#pragma once
#include <string>
#include <vector>

using namespace std;


int runEcho(const vector<string>& args);
int runPwd();
int runCd(const vector<string>& args);
int runLs(const vector<string>& args);
void runBG(const vector<string>& args);
void runPinfo(const vector<string>& args);
bool runSearch(const string &filename, const string &path);
void runIOredir(const vector<string>& args);
void runPipeline(const vector<string>& args);
void runHistory(const vector<string>& args);
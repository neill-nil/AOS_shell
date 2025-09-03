#pragma once
#include <string>
#include <vector>

using namespace std;


int runEcho(const vector<string>& args);
int runPwd();
int runCd(const vector<string>& args);
int runLs(const vector<string>& args);
void runGedit(const vector<string>& args, bool bg);
void runPinfo(const vector<string>& args);
bool runSearch(const string &filename, const string &path);
void runIOredir(const vector<string>& args);
void runHistory(const vector<string>& args);
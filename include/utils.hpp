#pragma once
#include <string>
#include <vector>

using namespace std;

vector<string> tokenize(const string& line);
vector<string> splitCommands(const string &givenCmd);
void checkSignalInt(int signo);
void checkSignalStp(int signo);
void handleError(const string &cmd, const string &customMsg = "Not found");
string readFile(const string &path);
char* autofill_cmd(const char* prefix, int state);
char* autofill_filedir(const char* prefix, int state);
void addCmdToHistory(string& cmd);


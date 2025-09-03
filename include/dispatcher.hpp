#pragma once
#include <string>
#include <vector>

using namespace std;

string getShellPrompt();
void runCommand(vector<string> &args, string fullCmd);


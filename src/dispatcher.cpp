#include <utils.hpp>
#include <commands.hpp>
#include <pwd.h>
#include <limits.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <iostream>
using namespace std;



string getHost() {
        char buffer[HOST_NAME_MAX];
        if (gethostname(buffer, sizeof(buffer)) != 0) {
            return "unknown";
        }
        return string(buffer);
}


string getShellPrompt() {

    struct passwd *pw = getpwuid(getuid());
    string user = (pw ? pw->pw_name : "unknown");


    string host = getHost();

    // Get current working directory
    char cwd[PATH_MAX];
    string cwd_str;
    if (getcwd(cwd, sizeof(cwd))) {
        cwd_str = string(cwd);
    } else {
        cwd_str = "?";
    }

    string red   = "\033[1;31m";
    string blue  = "\033[1;34m";
    string reset = "\033[0m";
    
    string prompt = red + user + "@" + host + reset + ":" + blue + cwd_str + reset + "$ ";
    return prompt;
}

//
void runCommand(vector<string> &args, string fullCmd) {
    if (args.empty()) return;

    string cmd = args[0];
    bool backgroundCmd = false;
    if (args.back() == "&"){
        backgroundCmd=true;
        args.pop_back();
    }

    bool ioflag = false;
    bool pipeflag= false;
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i]=="<" || args[i]==">" || args[i]==">>") ioflag=true;
        else if (args[i]=="|") pipeflag=true;
    }
    if(ioflag){
        runIOredir(args);
    }
    else if(pipeflag){
        runPipeline(args);
    }
    else if(backgroundCmd){
        runBG(args);
    }
    else if (cmd == "cd") {
        runCd(args);
    }
    else if (cmd == "pwd") {
        runPwd();
    }
    else if (cmd == "echo") {
        runEcho(args);
    }
    else if (cmd == "ls") {
        runLs(args);
        cout<<"\n";
    }
    else if (cmd == "pinfo") {
        runPinfo(args);
        cout<<"\n";
    }
    else if (cmd == "search") {
            if (args.size() < 2) {
            handleError("search", "missing filename");
            return;
        }
        
        string filename = args[1];
        cout << (runSearch(filename, ".") ? "True\n" : "False\n");
        cout<<"\n";
    }
    else if (cmd == "history") {
        runHistory(args);
        cout<<"\n";
    }
    else if (cmd == "exit") { // change exit call
        exit(0);
    }
    else {
        handleError(cmd);
    }
    addCmdToHistory(fullCmd);
}
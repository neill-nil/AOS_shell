#include <dispatcher.hpp>
#include <utils.hpp>
#include "global.hpp"
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

using namespace std;

char** autofill(const char* prefix, int start, int end){
    if (start==0){
        return rl_completion_matches(prefix, autofill_cmd);
    }
    else return rl_completion_matches(prefix, autofill_filedir);
}


int main() {

    signal(SIGINT, checkSignalInt);  
    signal(SIGTSTP, checkSignalStp);

    initShellRoot();

    // Autofill
    rl_attempted_completion_function = autofill;

    using_history();  
    read_history(historyFile.c_str());

    while (true) {
        // Getting user details like user, hostname and cwd

        // cout << getShellPrompt();
        
        char* givenCmd;
        givenCmd = readline(getShellPrompt().c_str());
        
        if (!givenCmd) {
            cout << "\nExiting shell...\n";
            break;
        }


        string givenCmdstr = givenCmd;

        if (!givenCmdstr.empty()) {
            add_history(givenCmd);
            append_history(1, historyFile.c_str());
        }
        free(givenCmd);

        vector<string> commands = splitCommands(givenCmdstr);

        // run each command separately
        for (auto &cmd : commands) {
            vector<string> args = tokenize(cmd);
            runCommand(args, cmd);
        }
    }
    write_history(historyFile.c_str());
    return 0;
}

#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>

using namespace std;

const char* home = getenv("HOME");
string historyFile = string(home) + "/.customshell_history";

pid_t fg_pid = -1;  

void checkSignalInt(int signo) {
    if (fg_pid > 0) {
        kill(-fg_pid, SIGINT); 
    }
}

void checkSignalStp(int signo) {
    if (fg_pid > 0) {
        kill(-fg_pid, SIGTSTP); 
        cout << "\nProcess " << fg_pid << " stopped\n";
    }
}


void handleError(const string &cmd, const string &customMsg = "Not found") {
    int err = errno;  
    if (err == 0) {
        cerr << "Shell Error: " << cmd << ": " << customMsg << "\n";
    } else {
        string prefix = "Shell Error: " + cmd;
        perror(prefix.c_str());
    }
}

vector<string> tokenize(const string& givenCmd) {
    vector<string> tokens;
    string token;
    bool inQuotes = false;

    for (size_t i = 0; i < givenCmd.size(); i++) {
        char c = givenCmd[i];

        if (c == '"') {
            inQuotes = !inQuotes; 
            continue; 
        }

        if (!inQuotes && isspace(c)) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        }
        else {
            token.push_back(c);
        }
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}


// Funtion to split commands by ; or \n...
vector<string> splitCommands(const string &givenCmd) {
    vector<string> commands;
    string cmd;

    for (size_t i = 0; i < givenCmd.size(); i++) {
        if (givenCmd[i] == ';' || givenCmd[i] == '\n') {
            if (!cmd.empty()) {
                commands.push_back(cmd);
                cmd.clear();
            }
        } 
        else cmd += givenCmd[i];
    }

    if (!cmd.empty()) {
        commands.push_back(cmd);
    }

    return commands;
}

string readFile(const string &path) {

    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) return "";

    char buffer[16* 1024];
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes <= 0) return "";

    buffer[bytes] = '\0';
    return string(buffer);
}

char* autofill_cmd(const char* prefix, int state){
    static size_t ind;
    static size_t size;

    static vector<string> commands = {"cd", "pwd", "ls", "echo", "cat", "search", "pinfo", "grep", "gedit", "sort"};

    if (state==0) {
        ind=0;
        size = strlen(prefix);
    }

    while (ind < commands.size()) {
        const string& cmd = commands[ind++];
        
        if (cmd.compare(0, size, prefix) == 0) {
            return strdup(cmd.c_str());  
        }
        
    }

    return nullptr;
}

char* autofill_filedir(const char* prefix, int state) {
    static vector<string> matches;
    static size_t ind;

    if (state == 0) {
        matches.clear();
        ind = 0;

        DIR* dir = opendir(".");
        if (!dir) return nullptr;

        struct dirent* entry;
        size_t size = strlen(prefix);

        while ((entry = readdir(dir)) != nullptr) {
            string name = entry->d_name;
            if (name.compare(0, size, prefix) == 0) {
                matches.push_back(name);
            }
        }
        closedir(dir);
    }

    if (ind < matches.size()) {
        return strdup(matches[ind++].c_str());
    }

    return nullptr;
}

void addCmdToHistory(string& cmd){


    vector<string> prevCmds;
    int hist_fd = open(historyFile.c_str(), O_RDWR | O_CREAT, 0644);

    if(hist_fd<0){
        handleError("add to history", "failed to open history file");
        return;
    }

    char buffer[16* 1024];
    ssize_t data = read(hist_fd, buffer, sizeof(buffer)-1);
    if(data){
        buffer[data] = '\0';

        string cur;
        for (ssize_t i = 0; i < data; i++) {
            if (buffer[i] == '\n') {
                if (!cur.empty()) prevCmds.push_back(cur);
                cur.clear();
            } 
            else {
                cur.push_back(buffer[i]);
            }
        }
        if (!cur.empty()) prevCmds.push_back(cur);
    }

    prevCmds.push_back(cmd);

    if (prevCmds.size()>20){
        prevCmds.erase(prevCmds.begin(), prevCmds.begin()+(prevCmds.size()-20));
    }

    ftruncate(hist_fd, 0);       
    lseek(hist_fd, 0, SEEK_SET); 

    for (auto &c : prevCmds) {
        string entry = c + "\n";
        write(hist_fd, entry.c_str(), entry.size());
    }

    close(hist_fd);

}
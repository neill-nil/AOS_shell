#include "commands.hpp"
#include "global.hpp"
#include "utils.hpp"
#include <unistd.h>   
#include <limits.h> 
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h> 
#include <fstream> 
#include <cstdlib>    
#include <cstring>    
#include <iomanip>
#include <iostream>

using namespace std;

const int MAX_PATH_LEN = 1000;

string prevDir; // keeping track of previous directory

int runEcho(const vector<string>& args) {
    for (size_t i = 1; i < args.size(); i++) {
        cout << args[i];
        if (i + 1 < args.size()) cout << ' ';
    }
    cout << '\n';
    return 0;
}

int runPwd() {

    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer))) {
        cout << buffer << '\n';
        return 0;
    }
    handleError("pwd");
    return 1;
}



int runCd(const vector<string>& args) {
    const char* targetDir = nullptr;

    if (args.size() > 2) {
        handleError("cd");
        return 1;
    }

    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        handleError("cd");
        return 1;
    }
    string currentDir = cwd;

    if (args.size() == 1) {
        targetDir = getenv("HOME");
        if (!targetDir) targetDir = "/";
    } else {
        string arg = args[1];

        if (arg == ".") {
            targetDir = currentDir.c_str(); 
        }
        else if (arg == "..") {
            targetDir = ".."; 
        }
        else if (arg == "-") {
            if (prevDir.empty()) {
                handleError("cd");
                return 1;
            }
            targetDir = prevDir.c_str();
        }
        else if (arg == "~") {
            targetDir = getenv("HOME");
            if (!targetDir) targetDir = "/";
        }
        else {
            targetDir = arg.c_str(); 
        }
    }

    if (chdir(targetDir) != 0) {
        handleError("cd");
        return 1;
    }

    // Update previous directory
    prevDir = currentDir;


    return 0;
}

void printPermissions(mode_t mode) {
    cout << ((S_ISDIR(mode)) ? "d" : "-");
    cout << ((mode & S_IRUSR) ? "r" : "-");
    cout << ((mode & S_IWUSR) ? "w" : "-");
    cout << ((mode & S_IXUSR) ? "x" : "-");
    cout << ((mode & S_IRGRP) ? "r" : "-");
    cout << ((mode & S_IWGRP) ? "w" : "-");
    cout << ((mode & S_IXGRP) ? "x" : "-");
    cout << ((mode & S_IROTH) ? "r" : "-");
    cout << ((mode & S_IWOTH) ? "w" : "-");
    cout << ((mode & S_IXOTH) ? "x" : "-");
}

int runLs(const vector<string>& args) {
    bool showHidden = false, showPermissions = false, showBoth = false;
    vector<string> paths;

    for (size_t i = 1; i < args.size(); i++) {
        string arg = args[i];
        if (arg == "-a") showHidden = true;
        else if (arg == "-l") showPermissions = true;
        else if (arg == "-al" || arg == "-la") {
            showBoth = true;
            showHidden = true;
            showPermissions = true;
        }
        else if (!arg.empty() && arg[0] != '-') {
            paths.push_back(arg);
        }
        else {
            handleError("ls");
            return 1;
        }
    }

    if (paths.empty()) paths.push_back(".");

    for (size_t i = 0; i < paths.size(); i++) {
        string path = paths[i];
        DIR* dir = opendir(path.c_str());
        if (!dir) {
            handleError("ls");
            continue;
        }

        if (paths.size() > 1) {
            cout << path << "  ";
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string name = entry->d_name;

            if (name[0] == '.' && !showHidden && !showBoth) continue;

            if (showPermissions || showBoth) {
                struct stat st;
                string fullPath = path + "/" + name;
                if (stat(fullPath.c_str(), &st) == -1) {
                    handleError("ls");
                    continue;
                }

                // permissions
                printPermissions(st.st_mode);
                cout << " ";

                // hard links
                cout << setw(2) << st.st_nlink << " ";

                // owner and group
                struct passwd* pw = getpwuid(st.st_uid);
                struct group* gr = getgrgid(st.st_gid);
                cout << (pw ? pw->pw_name : "?") << " ";
                cout << (gr ? gr->gr_name : "?") << " ";

                // file size
                cout << setw(6) << st.st_size << " ";

                // file name
                cout << name << "  ";
            }
            else {
                cout << name << "  ";
            }
        }
        closedir(dir);

        if (i < paths.size() - 1) cout << "\n"; 
    }

    return 0;
}


void runGedit(const vector<string>& args, bool bg){
    
    pid_t pid = fork();
    if (pid==-1){
        handleError("fork");
    }

    vector<char *> cargs;
    for (auto &a:args){
        cargs.push_back(const_cast<char *>(a.c_str()));
    }
    cargs.push_back(nullptr);

    //child process
    if(pid==0){
        execvp("gedit", cargs.data());
        handleError("execvp");
        _exit(1);
    }
    else{
        if(bg){
            cout<<"PID: "<<pid<<"\n";
        }
        else{
            int status;
            waitpid(pid, &status, 0);
        }
    }
    return;


}

void runPinfo(const vector<string> &args){
    int pid;
    if (args.size()>1){
        pid = stoi(args[1]);
    }
    else pid=getpid();

    string statPath = "/proc/"+ to_string(pid)+ "/stat";
    string statusPath = "/proc/"+ to_string(pid)+ "/status";
    string exePath = "/proc/"+ to_string(pid)+ "/exe";

    string statContent = readFile(statPath);
    string statusContent = readFile(statusPath);

    if(!statContent.c_str() || !statusContent.c_str()) {
        handleError("pinfo", "unable to find process information");
        return;
    }

    // get process status

    stringstream ss(statContent);

    int pidR, parentPid, pgrp, session, tty_nr, fgpid;
    string comm;
    char st;

    ss>>pidR >> comm >> st >> parentPid >> pgrp >> session >> tty_nr >> fgpid;

    string state = string(1, st);
    if(pgrp == fgpid) state+="+";

    //get process vm
    string identifier;
    string memUsage = "N/A";
    stringstream ms(statusContent);
    while (ms >> identifier) {
        if (identifier == "VmSize:") {
            ms >> memUsage;
            break;
        }
    }
    
    //get exe path
    char exeFullPath[MAX_PATH_LEN];
    size_t pathLen = readlink(exePath.c_str(), exeFullPath, sizeof(exeFullPath)-1);
    string exeStrPath = (pathLen==-1)? "N/A":string(exeFullPath, pathLen);


    cout<<"PID -- "<< pid<<"\n";
    cout<<"Process Status -- "<<state<<"\n";
    cout<<"memory -- "<<memUsage<<"\n";
    cout<<"Executable Path -- "<< exeStrPath<<"\n";

}

bool runSearch(const string &filename, const string &path){
    
    DIR* dir = opendir(path.c_str());
    if (!dir) handleError("search");

    struct dirent* file;
    while ((file = readdir(dir)) != nullptr) {
        string fname = file->d_name;
    
        if (fname == "." || fname == "..") continue;

        if (filename == fname) {
            closedir(dir);
            return true;
        }
        string updated_path = path+  "/" + fname;
        struct stat st;
        // recursion if directory
        if (stat(updated_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            if (runSearch(filename, updated_path)) {
                closedir(dir);
                return true;
            }
        }
    }
    closedir(dir);
    return false;


}

void runIOredir(const vector<string>& args){
    string mainCmd = args[0];
    string outFile;
    string inFile;
    int in_fd=-1, out_fd=-1;
    
    vector<char*> exec_args;
    for (size_t i = 0; i < args.size(); i++){
        if (args[i]=="<"){
            inFile = (i + 1 >= args.size())?"": args[i+1];
            if (inFile=="") handleError("I/O Redirection", "No Input file provided");
            in_fd = open(inFile.c_str(), O_RDONLY);
            if (in_fd<0) handleError("I/O Redirection", "Unable to open Input file");
            i++;

        }
        else if(args[i]==">" || args[i]==">>"){
            outFile = (i + 1 >= args.size())?"": args[i+1];
            if (outFile=="") handleError("I/O Redirection", "No Output file provided");

            if (args[i]==">") out_fd = open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            else out_fd = open(outFile.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
            
            if (out_fd<0) handleError("I/O Redirection", "Unable to open Output file");
            i++;
        }
        else exec_args.push_back(strdup(args[i].c_str()));
        
    }
    exec_args.push_back(nullptr);

    pid_t pid = fork();

    // Execute inside child process
    if (pid==0){
        if (in_fd!=-1){
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd!=-1){
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        execvp(mainCmd.c_str(), exec_args.data());
        _exit(1);
    }
    else if (pid>0){
        wait(nullptr);
    }
    else handleError("I/O Redirection");

    for (char* arg : exec_args) free(arg);
}


void runHistory(const vector<string>& args){

    

    int hist_fd = open(historyFile.c_str(), O_RDONLY);
    if(hist_fd<0){
        handleError("history", "failed to open history file");
        return;
    }
    char buffer[16* 1024];
    ssize_t data = read(hist_fd, buffer, sizeof(buffer)-1);
    close(hist_fd);

    if(data==0) return;

    buffer[data] = '\0';
    string cmdHist(buffer);

    vector<string> lines;
    string line;
    for (char c : cmdHist) {
        if (c == '\n') {
            if (!line.empty()) lines.push_back(line);
            line.clear();
        } else {
            line.push_back(c);
        }
    }

    if (!line.empty()) lines.push_back(line);

    int limit=10;
    if(args.size()>1 && stoi(args[1])<=10){
        try {
            limit = stoi(args[1]);
        } catch (...) {
            handleError("history", "invalid number");
            return;
        }
    }

    limit = (limit<=lines.size())?limit:lines.size();

    for (size_t i = lines.size() - limit; i < lines.size(); i++) {
        cout << lines[i] << "\n";
    }


}

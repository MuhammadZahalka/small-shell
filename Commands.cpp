#include <unistd.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <cstdlib>

#define LOWERCASE_A 'a'
#define LOWERCASE_Z 'z'
#define UPPERCASE_A 'A'
#define UPPERCASE_Z 'Z'
#define DIGIT_0 '0'
#define DIGIT_9 '9'

#define SIOCGIFADDR 0x8915      // Get IP address
#define SIOCGIFNETMASK 0x891b   // Get subnet mask


const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

bool ifElias(vector<pair<string,string>>& vec,string command){
    for(const auto& pair : vec){
        if(pair.first == command ){
            return true;
        }
    }
    return false;
}

string findx(vector<pair<string,string>>& vec,string command){
    for(const auto& pair : vec){
        if(pair.first == command ){
            return pair.second;/////second
        }
    }
    return "fuch firas";
}

string nonQuotes(const string& str) {
    string non_qoutes;
    int length = str.length();
    if (length >= 2 && str.back() == '\'' && str.front() == '\''){
        non_qoutes = str.substr(1, str.length() - 2);
    }
    else {
        non_qoutes = str;
    }
    return non_qoutes;
}


string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

Command::Command(const char *cmd_line) {
    for (int i = 0; i < COMMAND_MAX_ARGS + 1; i++)
        ArgsCommand[i] = nullptr;

    int ArgsNumber =  _parseCommandLine(cmd_line, ArgsCommand);

    command = string(cmd_line);

    numOfArgs = ArgsNumber;
}

// TODO: Add your implementation for classes in Commands.h

JobsList::JobsList(){}
JobsList::~JobsList(){}

SmallShell::SmallShell():prompt("smash"),jobs(JobsList()),foregroundPid(-1) {
    pid = getpid();
    char* buffer = getcwd(NULL,0);
    if (!buffer){
        perror("smash error: getcwd failed");
        return;
    }
    path = buffer;
    free(buffer);
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/

char* convertToNonconst(const char *cmd_line){
    if (!cmd_line) {
        return nullptr; // Handle null input safely
    }

    // Allocate memory for the new string
    size_t length = strlen(cmd_line);
    char* nonConstString = new char[length + 1]; // +1 for the null terminator

    // Copy the content of cmd_line into the new string
    strcpy(nonConstString, cmd_line);

    return nonConstString;
}

Command *SmallShell::CreateCommand(const char *cmd_line) {
    // For example:
    SmallShell& shell = SmallShell::getInstance();
    JobsList* work = shell.getJobsPointer();

    string cmd_s = _trim(string(cmd_line));
    //char* cmd_s = (char*)str.data();
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    string alia = string(cmd_line);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////
//
    char* line = convertToNonconst(cmd_line);
    if (ifElias(alias, firstWord)) {
        // Trim the command line for clean processing
        string trimmed_line = _trim(string(cmd_line));
        int pos = trimmed_line.find_first_not_of(WHITESPACE, cmd_s.find_first_of(" \n"));

        // Adjust the line pointer to skip the processed part
        line += pos;

        // Build the new command string
        string alias_value = nonQuotes(findx(alias, firstWord));
        string updated_cmd = _trim(alias_value + " " + string(line));

        // Update the command and first word
        cmd_s = updated_cmd;
        firstWord = updated_cmd.substr(0, updated_cmd.find_first_of(" \n"));

        // Allocate and copy the full updated command to 'line'
        line = new char[updated_cmd.size() + 1];
        strcpy(line, updated_cmd.c_str());
    }



    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (cmd_s.find('>') != string::npos && firstWord != "alias") {
        return new RedirectionCommand(line);
    }
    else if (cmd_s.find('|') != string::npos && firstWord != "alias") {
        return new PipeCommand(line);
    }

    if (firstWord.compare("chprompt") == 0) {
        _removeBackgroundSign(line);
        return new ChpromptCommand(line);
    }
    else if (firstWord.compare("showpid") == 0) {
        _removeBackgroundSign(line);
        return new ShowPidCommand(line);
    }
    else if(firstWord.compare("pwd") == 0){
        _removeBackgroundSign(line);
        return new GetCurrDirCommand(line);
    }
    else if(firstWord.compare("cd") == 0){
        _removeBackgroundSign(line);
        return new ChangeDirCommand(line);
    }
    else if(firstWord.compare("jobs")==0){
        _removeBackgroundSign(line);
        return new JobsCommand(line,work);
    }
    else if(firstWord.compare("fg")==0){
        _removeBackgroundSign(line);
        return new ForegroundCommand(line,work);
    }
    else if(firstWord.compare("quit")==0){
        _removeBackgroundSign(line);
        return new QuitCommand(line,work);
    }
    else if(firstWord.compare("kill")==0){
        _removeBackgroundSign(line);
        return new KillCommand(line,work);
    }
    else if(firstWord.compare("alias")==0){
        _removeBackgroundSign(line);
        return new aliasCommand(line);
    }
    else if(firstWord.compare("unalias")==0){
        _removeBackgroundSign(line);
        return new unaliasCommand(line);
    }
    else if(firstWord.compare("whoami")==0){
        _removeBackgroundSign(line);
        return new WhoAmICommand(line);
    }
    else{
        return new ExternalCommand(line, alia);
    }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    Command* cmd = CreateCommand(cmd_line);
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

bool checkArguments(char* ArgsCommand[COMMAND_MAX_ARGS+1]){
    if (!ArgsCommand[1] || !ArgsCommand[2]) {
        return false;
    }
    int signalNumber,jobNumber;
    try{
        string signal = ArgsCommand[1];
        string job = ArgsCommand[2];

        signalNumber = stoi(signal);
        jobNumber = stoi(job);
        if (signalNumber < 0 && jobNumber > 0) {
            return true;
        }
    }catch(invalid_argument){
        return false;
    }
    return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------------- aliasRelated functions-----------------------------------------------
string commandWord(const string& line) {
    int pos = line.find('=');
    if (pos == string::npos || pos + 1 >= line.length()) {
        return "";
    }
    return line.substr(pos + 1);
}

string newName(const string& line) {
    int pos = line.find('=');
    if (pos == string::npos) {
        return line;
    }
    return line.substr(0, pos);
}
void SplitCommand(char* ArgsCommand[COMMAND_MAX_ARGS + 1]) {
    string concatenatedArgs;
    for (int i = 1; i < COMMAND_MAX_ARGS + 1; i++) {
        if (ArgsCommand[i]) {
            concatenatedArgs += ArgsCommand[i];
            concatenatedArgs += " ";
        }
    }
    if (!concatenatedArgs.empty() && concatenatedArgs.back() == ' ') {
        concatenatedArgs.pop_back();
    }
    string trimmedArgs = _trim(concatenatedArgs);
    string afterEqual = commandWord(trimmedArgs);
    string preEqual = newName(trimmedArgs);
    ArgsCommand[1] = strdup(preEqual.c_str());
    ArgsCommand[2] = strdup(afterEqual.c_str());
}

////----------------------------------------------------------------------------

// void SplitCommand(char* ArgsCommand[COMMAND_MAX_ARGS + 1], string command) {
//     //cout<<command<<endl;
// //    string concatenatedArgs;
// //    string str = ArgsCommand[0];
// //    for (int i = 1; i < COMMAND_MAX_ARGS + 1; i++) {
// //        if (ArgsCommand[i]) {
// //            cout<<ArgsCommand[i]<<endl;
// //            concatenatedArgs += std::string(ArgsCommand[i]);
// //            concatenatedArgs += " ";
// //        }
// //    }

// //  cout<<"trim ="<< concatenatedArgs<<endl;


// //    if (!concatenatedArgs.empty() && concatenatedArgs.back() == ' ') {
// //        concatenatedArgs.pop_back();
// //    };

//     if (!command.empty() && command.back() == ' ') {
//         command.pop_back();
//     };

//     //cout<<"trim ="<< concatenatedArgs<<endl;

//     //string afterEqual = commandWord(concatenatedArgs);
//     //string preEqual = newName(concatenatedArgs);

//     string afterEqual = commandWord(command);
//     string preEqual = newName(command);

//     // cout<<"pre Equal ="<< preEqual<<endl;
//     // cout<<"after Equal ="<< afterEqual<<endl;

//     ArgsCommand[1] = strdup(preEqual.c_str());
//     ArgsCommand[2] = strdup(afterEqual.c_str());

// }


bool validName(string newName){
    ////// if the name is valid
    int size = newName.size();
    for(int i =0 ; i < size; i++){
        if(((newName[i] >= LOWERCASE_A && newName[i]<=LOWERCASE_Z)||
            (newName[i] >= UPPERCASE_A && newName[i] <= UPPERCASE_Z)||
            (newName[i] >= DIGIT_0 && newName[i] <= DIGIT_9) || newName[i] == '_'))
            return true;
    }
    return false;
}

bool isExist(string newName) {
    if ((newName == "chprompt") || (newName == "showpid") || (newName == "pwd") || (newName == "cd") ||
        (newName == "jobs") || (newName == "fg") || (newName == "quit") || (newName == "kill") ||
        (newName == "alias") || (newName == "unalias") || (newName == "listdir") || (newName == "whoami") ||
        (newName == "netinfo") || (newName == ">") || (newName == ">>") || (newName == "|")) {
        return true;
    }
    return false;
}

string checkExist(vector<pair<string, string>>& aliasVector, const string& command) {
    for (const auto& alias : aliasVector) {
        if (alias.first == command) {
            return alias.second;
        }
    }
    return "DOESNTEXIST";
}
void remove_pair(vector<pair<string, string>>& aliasVector, const string& aliasToRemove) {
    for (auto it = aliasVector.begin(); it != aliasVector.end(); ++it) {
        if (it->first == aliasToRemove) {
            aliasVector.erase(it);
            return;
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------executes-----------------------------------------------

/////////////ChpromptCommand
void ChpromptCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    if(getNumArgs() == 1){
        shell.setPrompt("smash");
    }
    else{
        shell.setPrompt(ArgsCommand[1]);
    }
}

////////////showpid Command
void ShowPidCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    shell.jobs.removeFinishedJobs();
    cout<<"smash pid is "<<shell.getPid()<<endl;
}

////////////pwd Command
void GetCurrDirCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    cout<<shell.getPath()<<endl;
}

////////////JobsCommand
void JobsCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    JobsList& jobs = shell.jobs;
    jobs.printJobsList();
}

////////////QuitCommand
void QuitCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    JobsList& list = shell.getJobs();
    if(numOfArgs > 1) {
        if (strcmp(ArgsCommand[1], "kill") == 0) {
            list.killAllJobs();
        }
    }
    exit(0);
}

///////////KillCommand
void KillCommand::execute() {
    if(!checkArguments(ArgsCommand)){
        cerr<<"smash error: kill: invalid arguments"<<endl;
        return;
    }
    if(numOfArgs != 3){
        cerr<<"smash error: kill: invalid arguments"<<endl;
        return;
    }

    int signalNumber = stoi(string(ArgsCommand[1])) * -1;
    int jobNumber = stoi(string(ArgsCommand[2]));

    SmallShell& shell= SmallShell::getInstance();
    JobsList& work= shell.getJobs();

    for(auto& job : work.getJob())
    {
        //work.printJobsList();
        if(job.id == jobNumber){
            cout << "signal number "<<signalNumber<<" was sent to pid "<<job.processId<<endl;
            //work.printJobsList();
            if(kill(job.processId,signalNumber)==-1){
                //work.printJobsList();
                perror("smash error: kill failed");
                return;
            }
            //work.printJobsList();
            return;
        }
    }
    cerr<<"smash error: kill: job-id "<< jobNumber <<" does not exist"<<endl;
    shell.getJobs().removeFinishedJobs();




    // if(numOfArgs!=3 ){
    //     cerr<<"smash error: kill: invalid arguments"<<endl;
    //     return;
    // }
    // if(!checkArguments(ArgsCommand)){
    //     cerr<<"smash error: kill: invalid arguments"<<endl;
    //     return;
    // }
    // SmallShell& current_shell=SmallShell::getInstance();
    // JobsList& temp=current_shell.getJobs();
    // int num1 = stoi(string(ArgsCommand[1])) * -1;
    // int num2 = stoi(string(ArgsCommand[2]));
    // for(auto it= temp.getJob().begin(); it!=(temp.getJob()).end();++it)
    // {
    //     if(it->id == num2){
    //         cout << "signal number "<<num1<<" was sent to pid "<<it->processId<<endl;
    //         if(kill(it->processId,num1)==-1){
    //             perror("smash error: kill failed");
    //             return;
    //         }
    //         return;
    //     }
    // }
    // cerr<<"smash error: kill: job-id "<<num2<<" does not exist"<<endl;

}
///////////cd execute
void ChangeDirCommand::execute() {
    SmallShell &shell = SmallShell::getInstance();

    int numberOfArgs = getNumArgs();
    if (numberOfArgs >= 3) {
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }
//    if (numberOfArgs == 1) {
//        cerr << "smash error: no args in cd" << endl;
//        return;
//    }
    char *directory = getcwd(nullptr, 0);
    if (!directory) {
        perror("smash error: getcwd failed");
        return;
    }
    const string &previousPath = shell.getPreviousPath();
    if (numberOfArgs == 2 && string(ArgsCommand[1]) == "-") {
        if (previousPath.empty()) {
            cerr << "smash error: cd: OLDPWD not set" << endl;
            free(directory);
            return;
        }
        if (chdir(previousPath.c_str()) == -1) {
            perror("smash error: chdir failed");
            free(directory);
            return;
        }
        shell.setPwd(previousPath);
        free(directory);
        return;
    }
    const char* newPath = ArgsCommand[1];
    if (chdir(newPath) == -1) {
        perror("smash error: chdir failed");
        free(directory);
        return;
    }
    char* newDirectory = getcwd(nullptr, 0);
//    if (!newDirectory) {
//        perror("smash error: getcwd failed after chdir");
//        free(directory);
//        return;
//    }
    shell.setPwd(newDirectory);
    free(directory);
    free(newDirectory);
}
void ForegroundCommand::execute() {
    SmallShell &shell = SmallShell::getInstance();
    JobsList &jobsList = shell.getJobs();
    jobsList.removeFinishedJobs();
    list<JobsList::JobEntry> jobsEntry = jobsList.getJob();
    bool flag = false;
    int numOfArgs = getNumArgs();

    if (numOfArgs == 1) {
        if (jobsEntry.empty()) {
            cerr << "smash error: fg: jobs list is empty" << endl;
            return;
        }
        int max = jobsList.getMaxId();
        JobsList::JobEntry &maxJob = jobsList.getMaxJob(max);
        int PIDMax = maxJob.processId;
        cout << maxJob.myCommand << " " << PIDMax << endl;
        shell.setForegroundPID(PIDMax);
        if (waitpid(PIDMax, NULL, 0) == -1) {
            perror("smash error: waitpid failed");
            shell.setForegroundPID(-1);
            return;
        }
        shell.setForegroundPID(-1);
        jobsList.removeJobById(maxJob.id);
    }
    else if (numOfArgs == 2) {
        string jobIDStr = string(ArgsCommand[1]);
//        if (!isInteger(jobIDStr)) {
//            cerr << "smash error: fg: invalid arguments" << endl;
//            return;
//        }
        for (int i = 0; i < jobIDStr.size(); ++i) {
            if (!isdigit(jobIDStr[i])) {
                cerr << "smash error: fg: invalid arguments" << endl;
                return;
            }
        }
        int jobID = stoi(jobIDStr);
        for (auto& job : jobsEntry) {
            if (job.id == jobID) {
                cout << job.myCommand << " " << job.processId << endl;
                shell.setForegroundPID(job.processId);

                if (waitpid(job.processId, nullptr, 0) == -1) {
                    perror("smash error: waitpid failed");
                    shell.setForegroundPID(-1);
                    return;
                }
                flag = true;
                shell.setForegroundPID(-1);
                jobsList.removeJobById(job.id);
                break;
            }
        }
        if (!flag) {
            cerr << "smash error: fg: job-id " << jobID << " does not exist" << endl;
        }
    }
    else {
        cerr << "smash error: fg: invalid arguments" << endl;
    }
    jobsList.removeFinishedJobs();
}
///////////////////alias execute
string& removeAfterQuote(string& str){

    // string result ="";

    int firstQuote = str.find('\'');
    if (firstQuote == string::npos) {
        ///cerr << "First single quote not found!" << std::endl;
        return str;
    }

    // Find the second occurrence of single quote
    size_t secondQuote = str.find('\'', firstQuote + 1);
    if (secondQuote == string::npos) {
        ///cerr << "Second single quote not found!" << std::endl;
        return str;
    }

    // Keep everything up to the second single quote (inclusive)
    str = str.substr(0, secondQuote + 1);
    return str;
}

void aliasCommand::execute() {

    //string str = ArgsCommand[1];
    string commandWithSpaces = commandWord(command);
    removeAfterQuote(commandWithSpaces);
    //cout << commandWithSpaces<< endl;

    /*
    string aliasCommand = ArgsCommand[0];

    int length = aliasCommand.length();

    string toPrint = command.substr(length);
    toPrint = _ltrim(toPrint);

    //cout<<toPrint<<endl;

    SplitCommand(ArgsCommand, toPrint);
    //SplitCommand(ArgsCommand, command);

    */

    //cout << ArgsCommand[2]<<endl;
    SplitCommand(ArgsCommand);

    SmallShell &shell = SmallShell::getInstance();

    int ArgsNumber = numOfArgs;
    if(ArgsNumber == 1){
        for(const auto& pair : shell.getVectorWithSpaces()){
            cout<<pair.first<<"="<<pair.second<<endl;
        }
        return;
    }

    string newName = ArgsCommand[1];
    string commandWord = ArgsCommand[2];




    //cout << commandWord << endl;



    //commandWord += "&";

    //cout << newName<<endl;
    //cout << commandWord<<endl;

     //string commandWord = commandWord(string(ArgsCommand[2]));

    if(!validName(newName)){
        cerr<<"smash error: alias: invalid alias format"<<endl;
        return;
    }

    if(isExist(newName)){
        cerr<<"smash error: alias: "<<newName<<" already exists or is a reserved command"<<endl;
        return ;
    }

    if(ifElias(shell.getAliasVector(), newName)){
        cerr<<"smash error: alias: "<<newName<<" already exists or is a reserved command"<<endl;
        return ;
    }


     shell.getAliasVector().push_back({newName, ArgsCommand[2]});
     shell.getVectorWithSpaces().push_back({newName, commandWithSpaces});

 }

///////////////////unalias execute

void unaliasCommand::execute(){
    SmallShell &shell = SmallShell::getInstance();
    int ArgsNumber = numOfArgs;
    if(ArgsNumber==1){
        cerr<<"smash error: unalias: not enough arguments"<<endl;
        return;
    }
    vector<pair<string,string>>& aliasVector = shell.getAliasVector();
    vector<pair<string,string>>& aliasWithSpacesVector = shell.getVectorWithSpaces();

    for(int i = 1 ; i < ArgsNumber ; i++){
        string aliasName = ArgsCommand[i];
        string check = checkExist(aliasVector,aliasName);
        if(check == "DOESNTEXIST"){
            cerr<<"smash error: unalias: "<<aliasName<<" alias does not exist"<<endl;
            return;
        }
//        pair<string,string> unalias =  {aliasName,check};
        remove_pair(aliasVector,aliasName);
        remove_pair(aliasWithSpacesVector, aliasName);
    }
}
////------------------------------------ External--------------------------------------------------------
// void ExternalCommand::execute() {
//     bool backGround = false;
//     bool Complex = false;
//     int argsNumber = numOfArgs;
//     for (int i = 0; i < argsNumber; ++i) {
//         string str = ArgsCommand[i];
//         if(str.find('?') != string::npos ||
//            str.find('*') != string::npos){
//             Complex = true;
//         }
//         int argLength = strlen(ArgsCommand[i]) - 1;
//         ////& in end of arg
//         if(ArgsCommand[i][argLength] == '&'){
//             backGround = true;
//             ArgsCommand[i][argLength] = '\0';
//         }
//         ////seperate &
//         if(strcmp(ArgsCommand[i],"&") == 0){
//             backGround = true;
//             ArgsCommand[i] = nullptr;
//         }

//     }
//     SmallShell& shell = SmallShell::getInstance();
//     JobsList& jobs = shell.getJobs();

//     int newJobId;
//     ///no jobs so new jobId must be 1
//     if(jobs.getJob().empty()){
//         newJobId = 1;
//     }
//         ///there are jobs so new jobId must be (max+1)
//     else{
//         int maxId = jobs.getMaxId();
//         newJobId = maxId + 1;
//     }

//     pid_t pid = fork();
//     if(pid < 0){
//         perror("smash error: fork failed");
//         return;
//     }

//         ///parent proccess
//     else if(pid > 0){
//         if(backGround){
//             jobs.addJob(this, false, newJobId, pid);
//         }
//         else{
//             shell.setForegroundPID(pid);
//             if(waitpid(pid,NULL,0) == -1){
//                 perror("smash error: waitpid failed");
//                 return;
//             }
//             shell.setForegroundPID(-1);
//         }
//     }
//         ///child proccess
//     else if (pid == 0) {
//         if(setpgrp()==-1){
//             perror("smash error: setpgrp failed");
//             return;
//         }
//         if(Complex){
//             if(execl("/bin/bash","-c",cmd_line, nullptr) == -1){
//                 perror("smash error: exec failed");
//                 exit(1);
//             }
//         }
//         else{
//             if(execvp(ArgsCommand[0], ArgsCommand) == -1){
//                 perror("smash error: execvp failed");
//                 exit(1);
//             }
//         }
//     }
// }

JobsList::JobEntry& get_max_job(JobsList& tmp_list)
{
    int max=0;
    int curr=0;
    list<JobsList::JobEntry>& tmp=tmp_list.getJob();
    JobsList::JobEntry* curr_max=new JobsList::JobEntry();
    for(auto it = tmp.begin(); it != tmp.end();++it)
    {
        curr=it->id;
        if(curr>max)
        {
            max=curr;
            *curr_max=*it;
        }
    }
    return *curr_max;
}

void ExternalCommand::execute() {
    bool is_complex = false;
    bool is_background = false;

    // Detect if the command is complex or should run in the background
    for (int i = 0; i < numOfArgs; ++i) {
        if (ArgsCommand[i] == nullptr) continue;
        if (strchr(ArgsCommand[i], '*') || strchr(ArgsCommand[i], '?')) {
            is_complex = true;
        }
        if (strcmp(ArgsCommand[i], "&") == 0) {
            is_background = true;
            ArgsCommand[i] = nullptr; // Remove '&' from arguments
        } else if (ArgsCommand[i][strlen(ArgsCommand[i]) - 1] == '&') {
            is_background = true;
            ArgsCommand[i][strlen(ArgsCommand[i]) - 1] = '\0'; // Remove '&' from the argument
        }
    }

    SmallShell& shell = SmallShell::getInstance();
    JobsList& jobsList = shell.getJobs();

    int job_id = jobsList.getJob().empty() ? 1 : get_max_job(jobsList).id + 1;

    pid_t pid = fork();
    if (pid < 0) {
        perror("smash error: fork failed");
        return;
    }

    if (pid == 0) {
        // Child process
        if (setpgrp() == -1) {
            perror("smash error: setpgrp failed");
            exit(1);
        }

        if (is_complex) {
            if (execl("/bin/bash", "bash", "-c", cmd_line, (char*)nullptr) == -1) {
                perror("smash error: exec failed");
                exit(1);
            }
        } else {
            if (execvp(ArgsCommand[0], ArgsCommand) == -1) {
                perror("smash error: execvp failed");
                exit(1);
            }
        }
    } else {
        // Parent process
        if (is_background) {
            jobsList.addJob(this,false, job_id, pid); // Add to jobs list
        } else {
            shell.setForegroundPID(pid); // Set foreground process
            if (waitpid(pid, NULL, WUNTRACED) == -1) {
                perror("smash error: waitpid failed");
            }
            shell.setForegroundPID(-1); // Reset foreground process ID
        }
    }
}

///------------------------------------- Special Commands-------------------------------------------------
void RedirectionCommand::execute() {
    string cmdLine = _trim(string(command));
    if (cmdLine[cmdLine.size() - 1] == '&') {
        cmdLine.pop_back();
    }
    int pos = 0;
    string fileHeader;
    string commandAsked;
    string symbol;
    if (cmdLine.find(">>") != std::string::npos) {
        symbol = ">>";
        pos = static_cast<int>(cmdLine.find(">>"));
        commandAsked = cmdLine.substr(0, pos);
        fileHeader = _trim(cmdLine.substr(pos + 2));
    } else {
        symbol = ">";
        pos = static_cast<int>(cmdLine.find(">"));
        commandAsked = cmdLine.substr(0, pos);
        fileHeader = _trim(cmdLine.substr(pos + 1));
    }

    SmallShell& shell = SmallShell::getInstance();
    pid_t pid = fork();

    if (pid == 0) {
        if (setpgrp() == -1) {
            perror("smash error: setpgrp failed");
            exit(1);
        }
        bool flag = (symbol == ">>");
        close(STDOUT_FILENO);
        int fileSys = open(fileHeader.c_str(), O_WRONLY | O_CREAT | (flag ? O_APPEND : O_TRUNC), 0664);
        if (fileSys == -1) {
            perror("smash error: open failed");
            exit(1);
        }
        if (fchmod(fileSys, 0664) == -1) {
            perror("smash error: fchmod failed");
            close(fileSys);
            exit(1);
        }

        char Argumentcommand[COMMAND_MAX_LENGTH + 1];
        strncpy(Argumentcommand, commandAsked.c_str(), COMMAND_MAX_LENGTH);
        Argumentcommand[COMMAND_MAX_LENGTH] = '\0';

        Command* theCommand = shell.CreateCommand(Argumentcommand);
        if (!theCommand) {
            perror("smash error: command creation failed");
            close(fileSys);
            exit(1);
        }
        theCommand->execute();
        close(fileSys);
        exit(0);

    } else if(pid < 0){
        perror("smash error: fork failed");
        return;
    }
    else if(pid > 0) {
        shell.setForegroundPID(pid);
        if (waitpid(pid, nullptr, 0) == -1) {
            perror("smash error: waitpid failed");
        }
        shell.setForegroundPID(-1);
    }
}

void PipeCommand::execute() {
    string command = getCommand();
    int signPosition = command.find('|');

    if (signPosition == string::npos) {
        perror("smash error: invalid pipe command");
        return;
    }

    int pipe_array[2];

    /// need? what & do
    bool StdErr = false;
    int length = command.size();
    if (signPosition + 1 < length && command[signPosition + 1] == '&'){
        StdErr = true;
    }

    if (StdErr){
        command.erase(signPosition, 2);
    } else {
        command.erase(signPosition, 1);
    }

//    if (pipePos + 1 < cmdLineStr.size() && cmdLineStr[pipePos + 1] == '&') {
//        redirectStdErr = true; // Detect |& for stderr redirection
//        cmdLineStr.erase(pipePos, 2); // Remove |& from the string
//    } else {
//        cmdLineStr.erase(pipePos, 1); // Remove | from the string
//    }

    //string command1 = command.substr(0, signPosition);
    string command1(command.begin(), command.begin() + signPosition);
    //string command2 = command.substr(signPosition);
    string command2(command.begin() + signPosition, command.end());


    SmallShell& shell = SmallShell::getInstance();

    if (pipe(pipe_array) == -1) {
        perror("smash error: pipe failed");
        return;
    }


    pid_t write = fork();
    if (write < 0) {
        perror("smash error: fork failed");
        return;
    }
    int duplicate;
    if (write == 0) {
        if (setpgrp() == -1) {
            perror("smash error: setpgrp failed");
            return;
        }

        close(pipe_array[0]);

        if (StdErr) {
            duplicate = dup2(pipe_array[1],2);
            if(duplicate == -1){
                perror("smash error: dup failed");
                close(pipe_array[0]);
                close(pipe_array[1]);
                return;
            }
        }
        else {
            duplicate = dup2(pipe_array[1],1);;
            if(duplicate == -1){
                perror("smash error: dup failed");
                close(pipe_array[0]);
                close(pipe_array[1]);
                return;
            }
        }

        close(pipe_array[1]);

        shell.pid = getpid();

        Command* command1T = shell.CreateCommand(command1.c_str());
        if(command1T != nullptr)
            command1T->execute();

        // execl("/bin/bash", "/bin/bash", "-c", command1.c_str(), nullptr);
        // perror("smash error: execl failed");
        exit(1);
    }

    // Fork the second process
    pid_t read = fork();
    if (read < 0) {
        perror("smash error: fork failed");
        return;
    }

    if (read == 0) {
        close(pipe_array[1]);
        duplicate = dup2(pipe_array[0],0);
        if(duplicate == -1){
            perror("smash error: dup failed");
            close(pipe_array[0]);
            close(pipe_array[1]);
            return;
        }
        close(pipe_array[0]);

        shell.pid = getpid();

        Command* command2T = shell.CreateCommand(command2.c_str());
        if(command2T != nullptr)
            command2T->execute();

        // execl("/bin/bash", "/bin/bash", "-c", command2.c_str(), nullptr);
        // perror("smash error: execl failed");

        exit(1);
    }

    shell.pid = getpid();
    close(pipe_array[0]);
    close(pipe_array[1]);

    // Wait for both child processes to finish
    if (waitpid(write, nullptr, 0) == -1 || (waitpid(read, nullptr, 0) == -1)) {
        perror("smash error: waitpid failed");
    }
}
void WhoAmICommand::execute() {
    char username[256];
    char homeDirectory[1024];
    uid_t uid = syscall(SYS_geteuid);
    int fd = syscall(SYS_open, "/etc/passwd", O_RDONLY);
    if (fd < 0) {
        std::cerr << "smash error: failed to open /etc/passwd: " << strerror(errno) << std::endl;
        return;
    }
    char buffer[4096];
    size_t bytesRead;
    std::string passwdEntry;
    while ((bytesRead = syscall(SYS_read, fd, buffer, sizeof(buffer))) > 0) {
        passwdEntry.append(buffer, bytesRead);
    }
    syscall(SYS_close, fd);
    if (bytesRead < 0) {
        std::cerr << "smash error: failed to read /etc/passwd: " << strerror(errno) << std::endl;
        return;
    }
    size_t pos = 0;
    size_t lineEnd;
    while ((lineEnd = passwdEntry.find('\n', pos)) != std::string::npos) {
        std::string line = passwdEntry.substr(pos, lineEnd - pos);
        pos = lineEnd + 1;
        size_t fieldStart = 0;
        size_t fieldEnd;
        std::string fields[7];
        int fieldIndex = 0;
        while ((fieldEnd = line.find(':', fieldStart)) != std::string::npos && fieldIndex < 7) {
            fields[fieldIndex++] = line.substr(fieldStart, fieldEnd - fieldStart);
            fieldStart = fieldEnd + 1;
        }
        fields[fieldIndex] = line.substr(fieldStart);
        if (fieldIndex >= 2 && std::stoi(fields[2]) == uid) {
            strncpy(username, fields[0].c_str(), sizeof(username) - 1);
            strncpy(homeDirectory, fields[5].c_str(), sizeof(homeDirectory) - 1);
            username[sizeof(username) - 1] = '\0';
            homeDirectory[sizeof(homeDirectory) - 1] = '\0';
            std::cout << username << " " << homeDirectory << std::endl;
            return;
        }
    }
    std::cerr << "smash error: user with UID " << uid << " not found in /etc/passwd" << std::endl;
}
///------------------------------------- jobsList functions-----------------------------------------------
void JobsList::printJobsList() {
    removeFinishedJobs();
    for (auto &job: jobsList) {
        job.printJob();
    }
}
void JobsList::addJob(ExternalCommand *cmd, bool isSignaled, int newJobId,int pid){
    removeFinishedJobs();
    SmallShell& shell = SmallShell::getInstance();
    JobsList& currentJobs = shell.getJobs();
    currentJobs.removeFinishedJobs();
    string cmd_line = cmd->get_cmd_line();
    currentJobs.getJob().push_back(JobEntry(cmd_line, newJobId, pid, isSignaled));
}
void JobsList::killAllJobs() {
    removeFinishedJobs();
    cout << "smash: sending SIGKILL signal to " << jobsList.size() << " jobs:" << endl;
    for (auto &job: jobsList) {
        if (kill(job.processId, 9) == -1) {
            perror("smash error: kill failed");
            return;
        }
        cout << job.processId << ": " << job.myCommand << endl;
    }
}

void JobsList::removeJobById(int jobId) {
    removeFinishedJobs();
    for (auto it = jobsList.begin(); it != jobsList.end();) {
        if (it->id == jobId) {
            it = jobsList.erase(it);
        } else {
            ++it;
        }
    }
}
//JobsList::JobEntry *JobsList::getLastJob(int lastJobId){
//    for (auto it = jobsList.begin(); it != jobsList.end(); ++it) {
//        if(it->id==lastJobId)
//            return &(*it);
//    }
//    return nullptr;
//}

//JobsList::JobEntry* JobsList::getLastStoppedJob(int jobId) {
//    for (auto it = jobsList.begin(); it != jobsList.end(); ++it) {
//        if(it->id==jobId)
//            return &(*it);
//    }
//    return nullptr;
//}

void JobsList::removeFinishedJobs() {
//    list<JobEntry>& tempList=jobsList;
//    for(auto it = tempList.begin(); it != tempList.end();){
//        int flag = waitpid(it->processId,nullptr,WNOHANG);
//        if (flag > 0)
//        {
//            it=tempList.erase(it);
//        }
//        else
//            ++it;
//    }
    // jobsList.remove_if([](const JobEntry &job) {
    //     return waitpid(job.processId, nullptr, WNOHANG) > 0;
    //     // return status > 0;
    // });

   for (auto it = jobsList.begin(); it != jobsList.end(); ) {
        if ((waitpid(it->processId, nullptr, WNOHANG) > 0)) {
            it = jobsList.erase(it); // Remove finished jobs
        } else {
            ++it;
        }
    }

}
list<JobsList::JobEntry> &JobsList::getJob() {
    removeFinishedJobs();
    return jobsList;
}
JobsList::JobEntry *JobsList::getJobById(int jobId) {
    removeFinishedJobs();
    for (auto it = jobsList.begin(); it != jobsList.end(); ++it) {
        if (it->id == jobId)
            return &(*it);
    }
    return nullptr;
}
int JobsList::getMaxId(){
    removeFinishedJobs();
    int max = 0;
    for (auto & job : jobsList) {
        if(job.id > max)
            max = job.id;
    }
    return max;
}
JobsList::JobEntry& JobsList::getMaxJob(int jobId) {
    removeFinishedJobs();
    JobsList::JobEntry* max = getJobById(jobId);
    return *max;
}
///-----------------------------------------smallShell functions---------------------------------------------------
int SmallShell::getPid(){
    return this->pid;
}

void SmallShell::setPid(int newPid){
    this->pid = newPid;
}

pid_t SmallShell::getForegroundPID(){
    return foregroundPid;
}

string& SmallShell::getPrompt(){
    return this->prompt;
}

void SmallShell::setPrompt(string newPrompt){
    this->prompt = newPrompt;
}

string& SmallShell::getPath(){
    return this->path;
}

void SmallShell::setPath(string newPath){
    this->path = newPath;
}
JobsList& SmallShell::getJobs(){
    return jobs;
}

JobsList* SmallShell::getJobsPointer(){
    return work;
}

string& SmallShell::getPreviousPath(){
    return this->previousPath;
}
void SmallShell::setPwd(string newPath){
    previousPath=path;
    path=newPath;
}
void SmallShell::setForegroundPID(int newfgPID){
    foregroundPid=newfgPID;
}
vector<pair<string,string>>& SmallShell::getAliasVector(){
    return alias;
}

vector<pair<string,string>>& SmallShell::getVectorWithSpaces(){
    return aliasWithSpaces;
}



#define AF_INET 2
#define SOCK_DGRAM 2
#define SIOCGIFADDR 0x8915
#define SIOCGIFNETMASK 0x891b

// Minimal definitions for network structures
struct in_addr {
    unsigned int s_addr;  // IPv4 address
};

struct sockaddr {
    unsigned short sa_family;  // Address family
    char sa_data[14];          // Address data
};

struct sockaddr_in {
    short sin_family;       // Address family (AF_INET)
    unsigned short sin_port; // Port number
    struct in_addr sin_addr; // IPv4 address
    char sin_zero[8];        // Padding
};

extern "C" long syscall(long number, ...);

// Convert binary IP address to dotted-decimal string
void convertBinaryToIpString(unsigned int ip, char *buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%u.%u.%u.%u",
             (ip & 0xFF),
             (ip >> 8) & 0xFF,
             (ip >> 16) & 0xFF,
             (ip >> 24) & 0xFF);
}

// Suggest available interfaces if the specified one does not exist
void suggestAvailableInterfaces() {
    const char *devFile = "/proc/net/dev";
    long devFd = syscall(SYS_open, devFile, O_RDONLY, 0);
    if (devFd >= 0) {
        char buffer[4096];
        long bytesRead = syscall(SYS_read, devFd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            char *line = strtok(buffer, "\n");
            const char *header = "Available interfaces:\n";
            syscall(SYS_write, 1, header, strlen(header));

            while ((line = strtok(nullptr, "\n"))) {
                char *colon = strchr(line, ':');
                if (colon) {
                    *colon = '\0';
                    char *interface = line + strspn(line, " \t");
                    if (strlen(interface) > 0) {
                        syscall(SYS_write, 1, "  - ", 4);
                        syscall(SYS_write, 1, interface, strlen(interface));
                        syscall(SYS_write, 1, "\n", 1);
                    }
                }
            }
        }
        syscall(SYS_close, devFd);
    }
}

void NetInfo::execute() {
    if (numOfArgs < 2) {
        const char *error = "smash error: netinfo: interface not specified\n";
        syscall(SYS_write, 2, error, strlen(error));
        return;
    }

    const char *interfaceName = ArgsCommand[1];
    char ipAddress[16] = "Not Found";
    char subnetMask[16] = "Not Found";
    char gateway[16] = "Not Found";
    char dnsServers[256] = "";

    // Step 1: Validate the interface name using /proc/net/dev
    const char *devFile = "/proc/net/dev";
    long devFd = syscall(SYS_open, devFile, O_RDONLY, 0);
    bool interfaceFound = false;

    if (devFd >= 0) {
        char buffer[4096];
        long bytesRead = syscall(SYS_read, devFd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            char *line = strtok(buffer, "\n");

            while ((line = strtok(nullptr, "\n"))) {
                if (strstr(line, interfaceName)) {
                    interfaceFound = true;
                    break;
                }
            }
        }
        syscall(SYS_close, devFd);
    }

    if (!interfaceFound) {
        char errorMsg[128];
        snprintf(errorMsg, sizeof(errorMsg),
                 "smash error: netinfo: interface %s does not exist\n",
                 interfaceName);
        syscall(SYS_write, 2, errorMsg, strlen(errorMsg));
        suggestAvailableInterfaces();
        return;
    }

    // Step 2: Fetch IP Address and Subnet Mask using `ioctl`
    long sockfd = syscall(SYS_socket, AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        syscall(SYS_write, 2, "Error: Could not create socket\n", 31);
        return;
    }

    struct ifreq {
        char ifr_name[16];
        struct sockaddr ifr_addr;
    } ifreq;

    memset(&ifreq, 0, sizeof(ifreq));
    strncpy(ifreq.ifr_name, interfaceName, sizeof(ifreq.ifr_name) - 1);

    // Get IP Address
    if (syscall(SYS_ioctl, sockfd, SIOCGIFADDR, &ifreq) == 0) {
        struct sockaddr_in *addr = (struct sockaddr_in *)&ifreq.ifr_addr;
        convertBinaryToIpString(addr->sin_addr.s_addr, ipAddress, sizeof(ipAddress));
    }

    // Get Subnet Mask
    if (syscall(SYS_ioctl, sockfd, SIOCGIFNETMASK, &ifreq) == 0) {
        struct sockaddr_in *addr = (struct sockaddr_in *)&ifreq.ifr_addr;
        convertBinaryToIpString(addr->sin_addr.s_addr, subnetMask, sizeof(subnetMask));
    }

    syscall(SYS_close, sockfd);

    // Step 3: Fetch Default Gateway from /proc/net/route
    const char *routeFile = "/proc/net/route";
    long routeFd = syscall(SYS_open, routeFile, O_RDONLY, 0);
    if (routeFd >= 0) {
        char buffer[4096];
        long bytesRead = syscall(SYS_read, routeFd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            char *line = strtok(buffer, "\n");

            while ((line = strtok(nullptr, "\n"))) {
                char iface[16], dest[16], gatewayHex[16];
                unsigned long flags;

                int matched = sscanf(line, "%15s %15s %15s %lx", iface, dest, gatewayHex, &flags);
                if (matched >= 3 && strcmp(iface, interfaceName) == 0 && strcmp(dest, "00000000") == 0) {
                    unsigned long gw = strtoul(gatewayHex, nullptr, 16);
                    convertBinaryToIpString(gw, gateway, sizeof(gateway));
                    break;
                }
            }
        }
        syscall(SYS_close, routeFd);
    }

    // Step 4: Fetch DNS Servers from /etc/resolv.conf
    const char *resolvFile = "/etc/resolv.conf";
    long resolvFd = syscall(SYS_open, resolvFile, O_RDONLY, 0);
    if (resolvFd >= 0) {
        char buffer[4096];
        long bytesRead = syscall(SYS_read, resolvFd, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            char *line = strtok(buffer, "\n");

            while (line) {
                if (strncmp(line, "nameserver", 10) == 0) {
                    strcat(dnsServers, line + 11);
                    strcat(dnsServers, " ");
                }
                line = strtok(nullptr, "\n");
            }
        }
        syscall(SYS_close, resolvFd);
    }

    // Step 5: Output results
    char output[512];
    snprintf(output, sizeof(output),
             "IP Address: %s\nSubnet Mask: %s\nDefault Gateway: %s\nDNS Servers: %s\n",
             ipAddress, subnetMask, gateway, dnsServers[0] ? dnsServers : "Not Found");
    syscall(SYS_write, 1, output, strlen(output));
}
////////////////////////////////////////////////////////////////////////////////////////////




void listDirectoryRecursiveRaw(const string &path, const string &indentation) {
    int dir_fd = syscall(SYS_open, path.c_str(), O_RDONLY | O_DIRECTORY);
    if (dir_fd < 0) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "smash error: cannot open directory '%s'", path.c_str());
        syscall(SYS_write, STDERR_FILENO, error_msg, strlen(error_msg));
        syscall(SYS_write, STDERR_FILENO, "\n", 1);
        return;
    }

    char buffer[4096];
    struct linux_dirent64 {
        ino64_t d_ino;
        off64_t d_off;
        unsigned short d_reclen;
        unsigned char d_type;
        char d_name[];
    };

    vector<string> directories;
    vector<string> files;

    int bytes_read;
    while ((bytes_read = syscall(SYS_getdents64, dir_fd, buffer, sizeof(buffer))) > 0) {
        int offset = 0;
        while (offset < bytes_read) {
            struct linux_dirent64 *entry = (struct linux_dirent64 *)(buffer + offset);
            string name = entry->d_name;
            if (name == "." || name == "..") {
                offset += entry->d_reclen;
                continue;
            }

            string fullPath = path + "/" + name;
            struct stat statbuf;
            if (syscall(SYS_stat, fullPath.c_str(), &statbuf) == -1) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "smash error: cannot stat '%s'", fullPath.c_str());
                syscall(SYS_write, STDERR_FILENO, error_msg, strlen(error_msg));
                syscall(SYS_write, STDERR_FILENO, "\n", 1);
                offset += entry->d_reclen;
                continue;
            }

            if (S_ISDIR(statbuf.st_mode)) {
                directories.push_back(name);
            } else {
                files.push_back(name);
            }

            offset += entry->d_reclen;
        }
    }

    if (bytes_read < 0) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "smash error: getdents64 failed for '%s'", path.c_str());
        syscall(SYS_write, STDERR_FILENO, error_msg, strlen(error_msg));
        syscall(SYS_write, STDERR_FILENO, "\n", 1);
    }

    syscall(SYS_close, dir_fd);

    sort(directories.begin(), directories.end());
    sort(files.begin(), files.end());

    for (const auto &directory : directories) {
        syscall(SYS_write, STDOUT_FILENO, (indentation + directory + "\n").c_str(), (indentation + directory + "\n").size());
        listDirectoryRecursiveRaw(path + "/" + directory, indentation + "\t");
    }

    for (const auto &file : files) {
        syscall(SYS_write, STDOUT_FILENO, (indentation + file + "\n").c_str(), (indentation + file + "\n").size());
    }
}

void ListDirCommand::execute() {
    if (getNumArgs() > 2) {
        const char *error_msg = "smash error: listdir: too many arguments\n";
        syscall(SYS_write, STDERR_FILENO, error_msg, strlen(error_msg));
        return;
    }

    string path;
    if (getNumArgs() == 1) {
        path = ".";  // Current directory
    } else {
        path = ArgsCommand[1];
    }

    struct stat statbuf;
    if (syscall(SYS_stat, path.c_str(), &statbuf) == -1 || !S_ISDIR(statbuf.st_mode)) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "smash error: cannot access directory '%s'", path.c_str());
        syscall(SYS_write, STDERR_FILENO, error_msg, strlen(error_msg));
        syscall(SYS_write, STDERR_FILENO, "\n", 1);
        return;
    }

    listDirectoryRecursiveRaw(path, "");
}
/////////////////////////////////////////////////////////////////////////////////
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <list>
#include <signal.h>
#include <memory>
#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

using namespace std;
class Command {
    // TODO: Add your data members

public:
    string command;
    int numOfArgs;
    char* ArgsCommand[COMMAND_MAX_ARGS+1];

    ~Command() noexcept {
        for(int i=0;i < COMMAND_MAX_ARGS+1;++i)
        {
            delete ArgsCommand[i];
        }
    }

    Command(const char *cmd_line);

    //virtual ~Command();

    virtual void execute() = 0;

    int getNumArgs(){
        return numOfArgs;
    }
    string& getCommand(){
        return command;
    }
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line): Command(cmd_line) {}

    virtual ~BuiltInCommand() {
    }
};

class ExternalCommand : public Command {
public:
    char* cmd_line;
    string aliasStr;
    ExternalCommand(const char *cmd_line, string aliasStr): Command(cmd_line),aliasStr(aliasStr){}

    virtual ~ExternalCommand() {
    }

    string& get_cmd_line(){
        return aliasStr;
    }

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line): Command(cmd_line){}

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line): Command(cmd_line){}

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class ChpromptCommand : public BuiltInCommand {
public:
    ChpromptCommand(const char *cmd_line):BuiltInCommand(cmd_line){}


    virtual ~ChpromptCommand() {}

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
    // TODO: Add your data members public:
public:
    ChangeDirCommand(const char *cmd_line):BuiltInCommand(cmd_line){}

    virtual ~ChangeDirCommand() {
    }

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line):BuiltInCommand(cmd_line){}

    virtual ~GetCurrDirCommand() {
    }

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line):BuiltInCommand(cmd_line){}

    virtual ~ShowPidCommand() {
    }

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
    // TODO: Add your data members public:
public:
    QuitCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line){}

    virtual ~QuitCommand() {
    }

    void execute() override;
};


class JobsList {
public:

    class JobEntry {
        // TODO: Add your data members
    public:
        string myCommand;
        int id;
        int processId;
        bool isSignaled;
        JobEntry(string command="nothing",int id=0,int pid=0, bool is_signaled=false):
                myCommand(command),id(id),processId(pid),isSignaled(is_signaled){} ///////////// nothing?

        void printJob() const
        {
            cout <<"["<< id <<"]" << " " << myCommand << endl ;
        }
    };

    // TODO: Add your data members
public:
    list<JobEntry> jobsList;
    int lastJobId;

    JobsList();

    ~JobsList();

    void addJob(ExternalCommand *cmd, bool isStopped, int newJobId,int pid);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    JobEntry& getMaxJob(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int lastJobId);

    JobEntry *getLastStoppedJob(int jobId);

    list<JobEntry>& getJob();

    int getMaxId();
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line){}

    virtual ~JobsCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line){}

    virtual ~KillCommand() {
    }

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs):BuiltInCommand(cmd_line){}

    virtual ~ForegroundCommand() {
    }

    void execute() override;
};

class ListDirCommand : public Command {
public:
    ListDirCommand(const char *cmd_line): Command(cmd_line){}

    virtual ~ListDirCommand() {
    }

    void execute() override;
};

class WhoAmICommand : public Command {
public:
    WhoAmICommand(const char *cmd_line): Command(cmd_line){}

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class NetInfo : public Command {
    // TODO: Add your data members
public:
    NetInfo(const char *cmd_line): Command(cmd_line){}

    virtual ~NetInfo() {
    }

    void execute() override;
};

class aliasCommand : public BuiltInCommand {
public:
    aliasCommand(const char *cmd_line):BuiltInCommand(cmd_line){}

    virtual ~aliasCommand() {
    }

    void execute() override;
};

class unaliasCommand : public BuiltInCommand {
public:
    unaliasCommand(const char *cmd_line):BuiltInCommand(cmd_line){}

    virtual ~unaliasCommand() {
    }

    void execute() override;
};


class SmallShell {
private:

    string prompt;
    string path;
    string previousPath;
    pid_t foregroundPid;
    vector<pair<string,string>> alias;
    vector<pair<string,string>> aliasWithSpaces;


    // TODO: Add your data members
    SmallShell();

public:
    int pid;
    JobsList jobs;
    JobsList *work;
    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);

    int getPid();

    pid_t getForegroundPID();

    void setPid(int newPid);

    string& getPrompt();

    void setPrompt(string newPrompt);

    string& getPath();

    void setPath(string newPath);

    JobsList& getJobs();

    JobsList* getJobsPointer();

    string& getPreviousPath();

    void setPwd(string newPath);

    void setForegroundPID(int newfgPID);

    vector<pair<string,string>>& getAliasVector();

    vector<pair<string,string>>& getVectorWithSpaces();

};

#endif //SMASH_COMMAND_H_

#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void printKilled(int killPid){
    cout << "smash: process "<< killPid <<" was killed"<<endl;
}

void CtrlC(){
    cout <<"smash: got ctrl-C"<<endl;
}



void ctrlCHandler(int sig_num) {
    SmallShell& shell = SmallShell::getInstance();
    int killPid= shell.getForegroundPID();

    CtrlC();

    if (killPid == -1) {
        return;
    }
    if (killPid == shell.getPid()){
        return;
    }
    if(kill(killPid,sig_num) == -1){
        perror("smash error: kill failed");
        return;
    }
    printKilled(killPid);
}

void alarmHandler(int sig_num) {
    ///TO DO
}
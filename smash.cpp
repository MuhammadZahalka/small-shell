#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char *argv[]) {
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }


    SmallShell &smash = SmallShell::getInstance();
    while (true) {
        std::cout << smash.getPrompt() << "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;

}
//#include <iostream>
//#include <signal.h>
//#include <memory>
//#include "Commands.h"
//#include "signals.h"

//int main(int argc, char *argv[]){
//    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
//        perror("smash error: failed to set ctrl-C handler");
//    }
//
//    SmallShell &smash = SmallShell::getInstance();
//    while (true)
//    {
//        std::cout << smash.getPrompt()<<"> ";
//        std::string cmd_line;
//        std::getline(std::cin, cmd_line);
//        unique_ptr<char[]> _line(new char[cmd_line.size() + 1]);
//        strcpy(_line.get(), cmd_line.c_str());
//        smash.get_current_jobs().removeFinishedJobs();
//        smash.executeCommand(_line.get());
//    }
//    return 0;
//}

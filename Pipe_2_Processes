/* 
 * File:   main.cpp
 * Author: murchmc
 * Copyright [2018] <murchmc>
 * Created on October 16, 2018, 11:17 AM
 */

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <iostream>   
#include <cstring> 
#include <vector>
#include <string>


using namespace std;

const int READ = 0;
const int WRITE = 1;
using StrVec = vector<string>;

// Method from my HW4 that i used to execute code for a string vector
void execute(StrVec cmds) {
        vector<char*> args;
        for (size_t i = 0; i < cmds.size(); i ++) {
            args.push_back(&cmds[i][0]);
        }
        args.push_back(nullptr);
        execvp(args[0], &args[0]);
    }

// Helper method that creates the pipe
void runPipe(StrVec argList1, StrVec argList2) {
    int pipefd[2];
    pipe(pipefd);
    
    int pid2, pid1 = fork();
    if (pid1 == 0) {
        // Child first process              
        close(pipefd[READ]);
        dup2(pipefd[WRITE], WRITE);
        execute(argList1);
    } else {               
        pid2 = fork();
        if (pid2 == 0) {
            // Child process #2                        
            close(pipefd[WRITE]);
            dup2(pipefd[READ], READ);
            execute(argList2);
        }
    }
    waitpid(pid1, nullptr, 0);
    close(pipefd[1]);
    waitpid(pid2, nullptr, 0);
}

int main(int argc, char** argv) { 
    // creates a vector for all the arguments, and then 
    // 2 separate vectors for each side of the pipe
    std::vector<std::string> argList, argList1, argList2;
    for (int i = 1; i < argc; i++) {
        argList.push_back(argv[i]);
    }
    // Separates the args into the first and second process
    bool next = false;
    for (auto arg : argList) {
        if (arg == "|") {
            next = true;
        } else if (!next) {
            argList1.push_back(arg);
        } else {
            argList2.push_back(arg);
        }
    }
    // I added this because my code was longer than 25 lines
    runPipe(argList1, argList2);
    return 0;
}   



/* File:   main.cpp
 * Author: Matthew
 * Copyright [2018] <murchmc>
 */
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include "murchmc_hw3.h"

using IntMap = std::unordered_map<int, int>;
using StringMap = std::unordered_map<int, std::string>;

// reads data and inserts the appropriate information into  the unordered maps.
void loadData(IntMap &p2p, StringMap &p2cmd, std::string& input) {
    std::ifstream in(input);
    std::string line;
    while (std::getline(in, line)) {
        std::string uid, cmd, cmd2, cmd3, dummy;
        int pid, ppid;
        std::istringstream is(line);
        is >> uid >> pid >> ppid >> dummy >> dummy >> dummy >> dummy;
        // Since cmd can be one word or more than 10 this makes sure it gets 
        // every part of the command
        while (is >> cmd2) {
            cmd += cmd2 + " ";
        }
        p2p[pid] = ppid;
        p2cmd[pid] = cmd;
    }
}

// recursive method that prints the data top-down instead of bottom up
void printData(int pid, IntMap p2p, StringMap p2cmd) {
        if (pid != 0) {
            printData(p2p[pid], p2p, p2cmd);
            std::cout << pid << "\t" 
                << p2p[pid] << "\t"
                << p2cmd[pid] << std::endl;
        }
}

int main(int argc, char** argv) {   
    IntMap p2p;
    StringMap p2cmd;
    std::string str= argv[1];
    loadData(p2p, p2cmd, str);
    const int pid = std::stoi(argv[2]);
    std::cout << "Process tree for PID: " << pid << std::endl;
    std::cout <<  "PID\tPPID\tCMD" << std::endl;
    printData(pid, p2p, p2cmd);
    return 0;
}


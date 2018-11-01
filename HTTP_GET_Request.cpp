/* 
 * File:   main.cpp
 * Author: murchmc
 * Copyright [2018] <murchmc>
 * Created on October 16, 2018, 11:17 AM
 */

#include <ext/stdio_filebuf.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <cstring> 
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <iomanip>


using namespace std;
using StrVec = vector<string>;
const char* transfer = "Transfer-Encoding: chunked\r\n";
const char* cls = "Connection: Close\r\n";
const int READ = 0;
const int WRITE = 1;

/* Copy and paste of method in lecture slide that 
 * decodes a string in command line
 * param str - the encoded string
 * returns decoded string
 */
std::string decode(std::string str) {
    size_t pos = 0;
    while ((pos = str.find_first_of("%+", pos)) != std::string::npos) {
        switch (str.at(pos)) {
            case'+': str.replace(pos, 1, " ");
                break;
            case'%':
            {
                std::string hex = str.substr(pos + 1, 2);
                char ascii = std::stoi(hex, nullptr, 16);
                str.replace(pos, 3, 1, ascii);
            }   //  decode '%xx'        
        }
        pos++;
    }
    return str;
}

/*
 * Executes a command
 * param argVec - string vector of arguments
 */
void execute(StrVec argVec) {
    vector<char*> args;
    cout << std::endl;
    for (size_t i = 0; i < argVec.size(); i++) {
        args.push_back(&argVec[i][0]);
    }
    args.push_back(nullptr);
    execvp(args[0], &args[0]);
}

/*
 * gets the input for cmd and args if the input is a command
 * param args - the string of the space seperated command and arguments
 * return - string vector of the input
 */
StrVec getInput(string cmds) {
    istringstream in(cmds);
    string word;
    StrVec args;
    while (in >> quoted(word)) {
        args.push_back(word);
    }
    return args;
}

/*
 * Puts the input into a string vector and counts the number of header lines
 * param numHeaders - number of header lines
 * return - string vector of the input
 */
StrVec getInput(int &num) {
    string line, word;
    StrVec args;
    bool endParagraph = false;
    while (getline(cin, line)) {
        if (line.length() == 1) {
            endParagraph = true;
        } else {
            istringstream in(line);
            while (in >> quoted(word)) {
                args.push_back(word);
            }
            num = num + 1;
        }
        if (!endParagraph)
            endParagraph = true;
    }
    num = num - 1;
    return args;
}

/* 
 * gets the MIME type for a file
 * param file - file with extension
 * return string - type of file
 */
string getMime(string file) {
    replace(file.begin(), file.end(), '.', ' ');
    istringstream in(file);
    string filename, extension;
    in >> filename >> extension;  // extracts the extension of the file

    if (extension == "html")
        return "text/html";
    else if (extension == "png")
        return "image/png";
    else if (extension == "jpeg")

        return "image/jpeg";
    else
        return "text/plain";
}

StrVec getCommands(string args) {
    StrVec cmd;
    args.erase(0, 14);

    // edits the line so it is a space separated list
    replace(args.begin(), args.end(), '&', ' ');
    replace(args.begin(), args.end(), '=', ' ');
    
    string decoded = decode(args);

    // gets rid of "cmd" and "args" from the input
    cmd = getInput(decoded);
    cmd.erase(cmd.begin());
    cmd.erase(cmd.begin() + 1);
    return cmd;
}

/* 
 * prints the chunked response
 * param valid - if file is good
 * param mimeType - extention of file
 * param num - number of http headers sent by client
 */
void chunkedResponse(bool valid, string filename, int num) {
    // 200 ok is valid
    if (valid) {
        cout << "HTTP/1.1 200 OK" << "\r\n";
    } else {  // 400 not found if not valid
        cout << "HTTP/1.1 404 Not Found" << "\r\n";
    }
    cout << "Content-Type: " << getMime(filename) << "\r\n";
    cout << transfer;
    cout << "X-Client-Header-Count: " << num << "\r\n";
    cout << cls;
    if (!valid) {
        cout << "\r\n2a\r\nThe following file was not found: " << filename 
                    << "\r\n0\r\n\r\n";
    }
}

/**
 * Prints the output of a file after chunkedResponse
 * @param in - file stream
 * @param valid - if the file is good
 * @param filename - the filename
 * @param num - number of headers givem
 */
void outPut(ifstream& in, bool valid, string filename, int num) {
    chunkedResponse(valid, filename, num);
    cout << "\r\n";
    string line;
    // prints line size in hexadecmial and then line
    while (getline(in, line)) {
        line.append("\n");
        cout << hex << line.size() << "\r\n";
        cout << line << "\r\n";
    }
    cout << "0\r\n\r\n";
}

/**
 * Runs the parent process
 * @param fd - the file descriptors
 * @param cmd - the commands in a vector of strings
 * @param num - num Headers
 */
void child(int fd[], StrVec cmd) {
    close(fd[READ]);
    dup2(fd[WRITE], WRITE);
    execute(cmd);
    close(fd[WRITE]);
}

/**
 * Runs the child process
 * @param fd - the file descriptors
 */
void parent(int fd[], int num) {
    close(fd[WRITE]); 
    __gnu_cxx::stdio_filebuf<char> fb(fd[READ], std::ios::in, 1);
    std::istream is(&fb);
    std::string line;
    chunkedResponse(true, "null", num);
    cout << "\n";
    int count = 0;  // For some reason my code was printing an empty line at 
                    // the start, so i do not print the first line
    while (getline(is, line)) {
        if (count > 0) {
            line.append("\n");
            cout << hex << line.size() << "\r\n";
            cout << line << "\r\n";
        }
        count++;
    }
    close(fd[READ]);
}

/*
 * Runs the commands and pipes the output to another process 
 * which prints the line size and line for each line
 * param args - the line as a string
 * param numHeaders - the number of headers given for getCommands method
 */
void runPipe(string args, int numHeaders) {    
    StrVec cmd = getCommands(args);
    int fd[2];
    int exitCode;
    
    pipe(fd);   
    pid_t pid = fork();
    if (pid == 0) {
        child(fd, cmd);
    } else {
        waitpid(pid, &exitCode, 0);
        parent(fd, numHeaders);
        cout << "10\r\n\r\n";
        cout << "Exit code: " << exitCode << "\r\n" <<  endl;
    }
    cout << "0\r\n\r\n";
}

int main(int argc, char** argv) {
    int numHeaders = 0;
    StrVec args = getInput(numHeaders);
    string arg1 = args[1];
    string word;
    bool command = false;
        if (arg1.size() >= 13) {
            if (arg1.substr(0, 13) == "/cgi-bin/exec") {
                runPipe(arg1, numHeaders);
                command = true;
            }
    }
    if (arg1 == "/") {
        chunkedResponse(false, getMime("index.html"), 0);
    } else if (!command) {
            arg1.erase(0, 1);
            ifstream file(arg1);
            if (file.good()) {
                outPut(file, true, arg1, numHeaders);
            } else {
            chunkedResponse(false, arg1, numHeaders);
        }  
    }
    return 0;
}



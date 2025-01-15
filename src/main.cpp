//-------------------------------------------------------------------------------------------------
//
//  llopenfiles     Dec-2024      Dennis Lang
//
//  Find open file handles. 
//  This program has two implementations, based off of code from these github projects
//    https://github.com/AlSch092/DetectOpenHandles
//    https://github.com/yihleego/handle-tools/tree/master/src
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2024
// https://landenlabs.com/
//
// ----- License ----
//
// Copyright (c) 2016 Dennis Lang
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#define VERSION "v1.3"

#include "ll_stdhdr.hpp"
#include "split.hpp"

#include <iostream>
#include <string>

size_t optionErrCnt = 0;
bool closeHandle = false;
bool terminateProcess = false;


#ifdef HAVE_WIN
#define strncasecmp _strnicmp
#endif

//
// Two possible implementations to find open files
//
#define USE_HANDLES_2

#ifdef USE_HANDLES_2
    #include "handles2.hpp"
    // Forked from  https://github.com/AlSch092/DetectOpenHandles
    #define HandlesT Handles2
#else
    #include "handles1.hpp"
    // Forked from https://github.com/yihleego/handle-tools/tree/master/src
    #define HandlesT Handles1
#endif

 

/* 
int DisplayHandles() {
    return HandlesT::FindHandles(allPids, allNames, FALSE);
}

int DisplayHandles(const PidList& findPids) {
    return HandlesT::FindHandles(findPids, allNames, FALSE);
}

int FindHandles(const PidList& findPids, const NameList& findNames) {
    return HandlesT::FindHandles(findPids, findNames, FALSE);
}

int CloseHandle(const PidList& findPids, const NameList& findNames) {
    return HandlesT::FindHandles(findPids, findNames, TRUE);
}
*/

void showHelp(const char* argv0) {
    std::cout << "List open files " VERSION  " " __DATE__ "\n"
        << argv0  
        << "\n"
        "  -pid=<pid>   ; Limit scan to this pid\n"
        "  -closeHandle ; When matching open handle found, try and close it\n"
        "  -terminate   ; When matching open handle found, try and terminate process\n"
        "\n"
        "  partOfFileName ... \n"
        "\n"
        "Examples:\n"
        "  llopenfiles file1.txt file2.txt \n"
        "  llopenfiles -pid=123 -pid=345 \n"
        "  llopenfiles -close my_text_file.txt \n"
        "  llopenfiles -terminate my_text_file.txt \n"
        "\n";
}

void showUnknown(const char* msg) {
    std::cerr << "Unknown option:" << msg << std::endl;
}

bool validOption(const char* validCmd, const char* possibleCmd, bool reportErr = true) {
    // Starts with validCmd else mark error
    size_t validLen = strlen(validCmd);
    size_t possibleLen = strlen(possibleCmd);

    if (strncasecmp(validCmd, possibleCmd, std::min(validLen, possibleLen)) == 0) {
        return true;
    }

    if (reportErr) {
        std::cerr <<  "Unknown option:'" << possibleCmd << "', expect:'" << validCmd  << std::endl;
        optionErrCnt++;
    }
    return false;
}

int main(int argc, const char* argv[]) {

    NameList findNames;
    PidList  findPids;

    bool doParseCmds = true;
    string endCmds = "--";
    for (int argn = 1; argn < argc; argn++) {
        string argStr(argv[argn]);
        if (*argv[argn] == '-' && doParseCmds) {
            Split cmdValue(argStr, "=", 2);

            if (cmdValue.size() == 2) {
                string cmd = cmdValue[0];
                string value = cmdValue[1];
                const char* cmdName = cmd.c_str() + 1;
                if (cmd.length() > 2 && *cmdName == '-') 
                    cmdName++;  // allow -- prefix on commands

                switch (*cmdName) {
                case 'p':
                    if (validOption(cmdName, "pid")) {
                        size_t pid = atoi(value.c_str());
                        if (pid != 0) {
                            findPids.insert(pid);
                        } else {
                            std::cerr << "Invalid pid, must be numeric, " << argStr << std::endl;
                            optionErrCnt++;
                        }
                    }
                    break;

                default:
                    showUnknown(argStr.c_str());
                }
            } else {
                const char* cmdName = argStr.c_str() + 1;
                if (argStr.length() > 2 && *cmdName == '-')
                    cmdName++;  // allow -- prefix on commands

                switch (*cmdName) {
                case '?':
                    showHelp(argv[0]);
                    return 0;
                case 'c':
                    closeHandle = validOption(cmdName, "closeHandle");
                    break;
                case 't':
                    closeHandle = terminateProcess = validOption(cmdName, "terminateProcess");
                    break;

                default:
                    showUnknown(argStr.c_str());
                }

                if (endCmds == argv[argn]) {
                    doParseCmds = false;
                }
            }
        } else {
            findNames.push_back(argStr);
        }
    }


    if (optionErrCnt != 0) {
        return -1;
    }

    return HandlesT::FindHandles(findPids, findNames, closeHandle, terminateProcess);
}

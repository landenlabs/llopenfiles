//-------------------------------------------------------------------------------------------------
//
//  llopenfiles     Dec-2024      Dennis Lang
//
//  Find open file handles
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2024
// https://landenlabs.com/
//
// This file is part of llopenfiles project.
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



#include <iostream>
#include "handles1.hpp"
#include "handles2.hpp"


//
// Two possible implementations to find open files
//

#if 2
// Forked from  https://github.com/AlSch092/DetectOpenHandles
#define HandlesT Handles2
#else
// Forked from https://github.com/yihleego/handle-tools/tree/master/src
#define HandlesT Handles1
#endif


int DisplayHandles() {
    return HandlesT::FindHandles(NULL, NULL, FALSE);
}

int DisplayHandles(ULONG pid) {
    return HandlesT::FindHandles(pid, NULL, FALSE);
}

int FindHandle(ULONG pid, const char* handleName) {
    return HandlesT::FindHandles(pid, handleName, FALSE);
}

int CloseHandle(ULONG pid, const char* handleName) {
    return HandlesT::FindHandles(pid, handleName, TRUE);
}

int main(int argc, const char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "-?")) {
        std::cout << "List open files\n" << argv[0] << " [pid | partOfFilename]\n";
        return 0;
    }
    if (argc == 1) {
        DisplayHandles();
    } else {
        unsigned pid = atoi(argv[1]);
        if (pid != 0)
            DisplayHandles(pid);
        else
            return FindHandle(0, argv[1]);
    }

    return 1;
}

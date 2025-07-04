//-------------------------------------------------------------------------------------------------
//
//  ll_stdhdr.hpp  Landen Labs Standard constants/typedefs/defines   Dennis Lang
//
//-------------------------------------------------------------------------------------------------
//
// Author: Dennis Lang - 2024
// https://landenlabs.com
//
// This file is part of lljson project.
//
// ----- License ----
//
// Copyright (c) 2024  Dennis Lang
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

#pragma once

typedef unsigned short  UShort;

#if defined(_WIN32) || defined(_WIN64)
    #define HAVE_WIN
    #define NOMINMAX
    #define _CRT_SECURE_NO_WARNINGS   // define before all includes
    // typedef unsigned long DWORD;
#else
    typedef unsigned int DWORD;
#endif

#include <string>
#include <vector>
#include <set>

using namespace std;        // use after including c++ headers
 
typedef unsigned long Pid_t;
typedef set<Pid_t> PidList;
typedef vector<string> NameList;
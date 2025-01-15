
// Handle1
// https://github.com/yihleego/handle-tools/tree/master/src


#pragma once

#include "ll_stdhdr.hpp"

#include <windows.h>
#include <vector>

namespace Handles1 {
    int FindHandles(const PidList& pids, const NameList& names, bool closeHandle, bool terminateProc);
}
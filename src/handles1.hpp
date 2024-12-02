
// Handle1
// https://github.com/yihleego/handle-tools/tree/master/src


#pragma once

#include <windows.h>

namespace Handles1 {
int FindHandles(ULONG pid, const char* handleName, bool closeHandle);
}
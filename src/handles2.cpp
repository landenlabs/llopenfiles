// Handle2
// 
//  Show open files for each process
//
// Based off https://github.com/AlSch092/DetectOpenHandles
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


#include "Handles2.hpp"
#include <Psapi.h>
#include <iostream>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

//-------------------------------------------------------------------------------------------------
// Returns vector SYSTEM_HANDLE active handles on system
 
std::vector<Handles2::SYSTEM_HANDLE2> Handles2::GetHandles() {
    NtQuerySystemInformationFunc NtQuerySystemInformation =
        (NtQuerySystemInformationFunc)GetProcAddress(
            GetModuleHandleA("ntdll.dll"), "NtQuerySystemInformation");
    if (!NtQuerySystemInformation) {
        printf("Could not get NtQuerySystemInformation function address @ "
               "Handles::GetHandles");
        return {};
    }

    ULONG bufferSize = 0x10000;
    PVOID buffer = nullptr;
    NTSTATUS status = 0;

    do {
        buffer = malloc(bufferSize);
        if (!buffer) {
            printf("Memory allocation failed @ Handles::GetHandles\n");
            return {};
        }

        status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)16, buffer,
                                          bufferSize, &bufferSize);
        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            free(buffer);
            bufferSize *= 2;
        } else if (!NT_SUCCESS(status)) {
            printf("NtQuerySystemInformation failed @ Handles::GetHandles\n");
            free(buffer);
            return {};
        }
    } while (status == STATUS_INFO_LENGTH_MISMATCH);

    PSYSTEM_HANDLE_INFORMATION2 handleInfo = (PSYSTEM_HANDLE_INFORMATION2)buffer;
    std::vector<SYSTEM_HANDLE2> handles(
        handleInfo->Handles, handleInfo->Handles + handleInfo->HandleCount);
    free(buffer);
    return handles;
}


//-------------------------------------------------------------------------------------------------
char nameBuffer[512];
char typeBuffer[128];
const unsigned OBJ_INFO_SIZE = 1024;

typedef struct {
    UNICODE_STRING Name;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef struct {
    UNICODE_STRING TypeName;
    ULONG Reserved[22]; // reserved for internal use
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

static bool query(_In_opt_ HANDLE handle,
    _In_ OBJECT_INFORMATION_CLASS objClass,
    _Out_writes_bytes_opt_(ObjectInformationLength) PVOID pObj,
    _In_ ULONG objSize ) {
    return NT_SUCCESS(NtQueryObject(handle, objClass, pObj, objSize, NULL));
}

template <typename TT> 
void WideToMb(const TT* inBuf,  char* outBuf, unsigned outSize) {
    WideCharToMultiByte( CP_ACP, 0, inBuf, -1, outBuf, outSize, NULL, NULL);
}

//-------------------------------------------------------------------------------------------------
bool Handles2::FindHandles(ULONG pid, const char* findName, bool closeHandle_NOT_IMPLEMENTED) {
    unsigned findNameLen = (findName != nullptr) ? strlen(findName) : 0;
    auto handles = GetHandles();

    POBJECT_NAME_INFORMATION pObjName =
        (POBJECT_NAME_INFORMATION)malloc(OBJ_INFO_SIZE);
    POBJECT_TYPE_INFORMATION pObjType =
        (POBJECT_TYPE_INFORMATION)malloc(OBJ_INFO_SIZE);
    char processName[MAX_PATH];
    HANDLE processHandle = NULL;
    ULONG lastPid = 0;
    size_t idx = 0;
    // std::cerr << "Total handles=" << handles.size() << std::endl;

    for (auto &handle : handles) {
        idx += 100;
        ULONG thisPid = handle.ProcessId;
        if (thisPid == 4 || handle.ObjectTypeNumber != 37)  // 37 = File
            continue;
        if (pid != 0 && pid != thisPid)
            continue;

        if (lastPid != thisPid) {
            if (lastPid != 0)
                CloseHandle(processHandle);
            lastPid = thisPid;
            processHandle = OpenProcess(
                PROCESS_ALL_ACCESS | PROCESS_DUP_HANDLE | PROCESS_SUSPEND_RESUME, FALSE, handle.ProcessId);
            if (GetModuleBaseNameA(processHandle, NULL, processName, sizeof(processName)) == 0) {
                processName[0] = '\0';  // Failed to get process name
            }
        }

        if (processHandle) {
            HANDLE dupHandle = INVALID_HANDLE_VALUE;

            if (DuplicateHandle(processHandle, (HANDLE)handle.Handle, GetCurrentProcess(), &dupHandle,
                    0, FALSE, DUPLICATE_SAME_ACCESS)) {

                if (GetFileType(dupHandle) == FILE_TYPE_DISK) {
                    typeBuffer[0] = '\0';
                    if (query(dupHandle, ObjectTypeInformation,pObjType, OBJ_INFO_SIZE)) {
                        WideToMb(pObjType->TypeName.Buffer, typeBuffer, min(sizeof(typeBuffer), pObjType->TypeName.Length));
                    }

                    nameBuffer[0] = '\0';
                    OBJECT_INFORMATION_CLASS ObjectNameInformation =  (OBJECT_INFORMATION_CLASS)1;
                    if (query(dupHandle, ObjectNameInformation, pObjName, OBJ_INFO_SIZE)) {
                        WideToMb(pObjName->Name.Buffer, nameBuffer, min(sizeof(nameBuffer), pObjName->Name.Length));
                    }

                    if (nameBuffer[0] != '\0' /* && typeBuffer[0] != '\0' */) {
                        if (findName == nullptr || strstr(nameBuffer, findName) != nullptr)
                            std::cout << processName << " | " << thisPid << "| " << nameBuffer << std::endl;
                    }

                    CloseHandle(dupHandle);
                }
            }
        } else {
            std::cerr << (idx/handles.size()) << "% \r";
        }
    }
    std::cerr << "Done\n";
    return false;
}
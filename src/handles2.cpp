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


#include "handles2.hpp"
#include "global.hpp"

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
        fprintf(stderr, "Could not get NtQuerySystemInformation function address @ "
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

        status = NtQuerySystemInformation(
            (SYSTEM_INFORMATION_CLASS)16, buffer, bufferSize, &bufferSize);
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

int WideToMb(const WCHAR* inBuf,  char* outBuf, size_t maxOutSize) {
    return WideCharToMultiByte( CP_ACP, 0, inBuf, -1, outBuf, (int)maxOutSize, NULL, NULL);
}

static bool contains(const NameList& names, const char* item) {
    for (const string& name : names) {
        if (strstr(item, name.c_str()) != nullptr)
            return true;
    }
    return false;
}


std::string& GetErrorMsg(std::string& msg, DWORD error)
{
    if (error != 0)
    {
        char* pszMessage;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR) & pszMessage,
            0, NULL);

        msg = pszMessage;
        LocalFree(pszMessage);
        int eolPos = (int)msg.find_first_of('\r');
        if (eolPos > 0)
           msg.resize(eolPos);
        msg.append(" ");
    }
    return msg;
}

bool enableDebugPrivledge() {

    //  https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debug-privilege?redirectedfrom=MSDN
    HANDLE hToken;
    HANDLE hProcess = GetCurrentProcess();
    if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        LUID luid;
        int success = LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid);
        if (!success) {
            fprintf(stderr, "LookupPrivilegeValue failed. Error: %d\n", GetLastError());
            return false;
        }

        TOKEN_PRIVILEGES tkp;
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Luid = luid;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        success = AdjustTokenPrivileges(hToken, false, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
        int err = GetLastError();
        if (!success) {
            std::string errMsg;
            GetErrorMsg(errMsg, err);
            fprintf(stderr, "Filed to adjust app priviledge %s\n", errMsg.c_str());
            return false;
        }
        else if (err == ERROR_NOT_ALL_ASSIGNED) {
            fprintf(stderr, "Debug privileges not enabled.\n");
            return false;
        }
    }
    return true;
}

//-------------------------------------------------------------------------------------------------
bool Handles2::FindHandles(
        const PidList& findPids, 
        const NameList& findNames, 
        bool closeHandle, 
        bool terminateProc) {

    // enableDebugPrivledge();

    auto handles = GetHandles();
     // std::cerr << "Total handles=" << handles.size() << std::endl;
    totalHandleCnt = (unsigned int)handles.size();

    POBJECT_NAME_INFORMATION pObjName = (POBJECT_NAME_INFORMATION)malloc(OBJ_INFO_SIZE);
    POBJECT_TYPE_INFORMATION pObjType = (POBJECT_TYPE_INFORMATION)malloc(OBJ_INFO_SIZE);
    char processName[MAX_PATH];
    HANDLE processHandle = NULL;
    ULONG lastPid = 0;
    size_t idx = 0;

    DWORD openProcAccess = PROCESS_DUP_HANDLE
        | PROCESS_ALL_ACCESS
        | PROCESS_SUSPEND_RESUME
        ;

    if (closeHandle)
        openProcAccess |= PROCESS_TERMINATE;

    for (auto &handle : handles) {
        idx += 100;
        ULONG thisPid = handle.ProcessId;

        if (thisPid == 4) //  || handle.ObjectTypeNumber != 37)  
            continue;

        if (!findPids.empty() && findPids.find((size_t)thisPid) == findPids.end())
            continue;

        //
        //  https://github.com/FuzzySecurity/PSKernel-Primitives/blob/master/Get-Handles.ps1
        // 
        //  https://www.codeproject.com/Tips/992827/Section-Handles-Enumeration-Extending-File-Unlocki
        //  Operating System	File object Type
        //    Windows 2000	      26	                 
        //    Windows XP	      28	                 
        //    Windows Vista	      28	                 
        //    Windows 7	          28	                 
        //    Windows 8	          31                     
        //    Windows 8.1	      30	                 
        //    Windows 10          37
        //    Windows 11          40
        //

        if (handle.ObjectTypeNumber < 25 || handle.ObjectTypeNumber > 41) {
            continue;
        }

        if (lastPid != thisPid) {
            if (lastPid != 0)
                CloseHandle(processHandle);
            lastPid = thisPid;

            processHandle = OpenProcess(openProcAccess, FALSE, handle.ProcessId);
            if (processHandle == 0) {
                failedOpenProcCnt++;
                if (verbose) {
                    std::string errMsg;
                    GetErrorMsg(errMsg, GetLastError());
                    fprintf(stderr, "Failed to open process %d, %s\n", thisPid, errMsg.c_str());
                }
            } else if (GetModuleBaseNameA(processHandle, NULL, processName, sizeof(processName)) == 0) {
                processName[0] = '\0';  // Failed to get process name
            } else {
                if (contains(findNames, processName)) {
                    std::cout << processName << ", " << thisPid << endl;
                    matchCnt++;
                }
                goodOpenProcCnt++;
            }
        }

        if (processHandle) {
            HANDLE dupHandle = INVALID_HANDLE_VALUE;

            // if (DuplicateHandle(processHandle, (HANDLE)handle.Handle, GetCurrentProcess(), &dupHandle, PROCESS_DUP_HANDLE, FALSE, 0)) {
            if (DuplicateHandle(processHandle, (HANDLE)handle.Handle, GetCurrentProcess(), &dupHandle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
               
                DWORD fileType = GetFileType(dupHandle);
                if (fileType == FILE_TYPE_DISK || fileType == FILE_TYPE_UNKNOWN) {

                    const size_t BUFR_SIZE = 512;
                    char nameBuffer[BUFR_SIZE];
                    char typeBuffer[BUFR_SIZE];
                    // char localPath[BUFR_SIZE];

                    typeBuffer[0] = '\0';
                    if (query(dupHandle, ObjectTypeInformation,pObjType, OBJ_INFO_SIZE)) {
                        unsigned int inLen = (unsigned int)pObjType->TypeName.Length;
                        WideToMb(pObjType->TypeName.Buffer, typeBuffer, sizeof(typeBuffer));
                    }

                    nameBuffer[0] = '\0';
                    unsigned int nameLen = GetFinalPathNameByHandleA(dupHandle, nameBuffer, sizeof(nameBuffer), FILE_NAME_NORMALIZED);
                    if (nameLen > 0) {
                        if (strncmp(nameBuffer, "\\\\?\\", 4) == 0)
                            memcpy(nameBuffer, nameBuffer+4, sizeof(nameBuffer));
                    } else if (nameLen == 0) { 
                        DWORD err = GetLastError(); // 6 = handle is invalid. 

                        OBJECT_INFORMATION_CLASS ObjectNameInformation =  (OBJECT_INFORMATION_CLASS)1;
                        if (query(dupHandle, ObjectNameInformation, pObjName, OBJ_INFO_SIZE)) {
                            unsigned int inLen = (unsigned int)pObjName->Name.Length;
                            WideToMb(pObjName->Name.Buffer, nameBuffer, sizeof(nameBuffer));
                       
                            /* 
                            // if (GetVolumePathNameA(nameBuffer, localPath, sizeof(localPath))) {
                            if (GetFullPathNameA(nameBuffer, sizeof(localPath), localPath, nullptr)) {
                                memcpy(nameBuffer, localPath, sizeof(nameBuffer));
                            }
                            */
                        }
                    }
                    
                    CloseHandle(dupHandle);

                    if (nameBuffer[0] != '\0' /* && typeBuffer[0] != '\0' */) {
                        fileHandleCnt++;

                        if (findNames.empty() || contains(findNames, nameBuffer)) {

                            std::cout << processName << ", " << thisPid;
                            if (verbose) {
                                // std::cout << " flags=" << handle.Flags;
                                std::cout << ", access=" << hex << handle.GrantedAccess << dec;
                                std::cout << ", type=" << (unsigned int)handle.ObjectTypeNumber;
                                std::cout << ", hnd=" << handle.Handle;
                            }

                            std::cout << ", " << nameBuffer;
                            std::cout << std::endl;
                            matchCnt++;

                            if (closeHandle && (!findPids.empty() || !findNames.empty()) ) {
                                HANDLE toCloseHnd;
                                DuplicateHandle(processHandle, (HANDLE)handle.Handle, GetCurrentProcess(), &toCloseHnd, 0, FALSE,  DUPLICATE_CLOSE_SOURCE);
                                if (0 == CloseHandle(toCloseHnd)) {
                                    std::cerr << "Failed to close handle on " << processName << "(" << thisPid << ") " << nameBuffer << std::endl;
                                    if (0 == TerminateProcess(processHandle, 0)) {
                                        std::cerr << "Failed to terminate " << processName << "(" << thisPid << ")\n";
                                    } else {
                                        std::cerr << "Terminate " << processName << "(" << thisPid << ")\n";
                                    }
                                } else {
                                    std::cout << "Closed handle on " << processName << "(" << thisPid << ") " << nameBuffer << std::endl;
                                }
                            }
                        }
                    }
                }
                else {
                    // if (verbose) std::cerr << "FileType=" << fileType << std::endl;
                }
            }
        } else {
            std::cerr << (idx/handles.size()) << "% \r";
        }
    }

    // std::cerr << "Done\n";
    return false;
}
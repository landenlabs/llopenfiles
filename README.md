#llrename  (Dec-2024)
DOS / Windows commnad line list openfiles 

Visit home website

[https://landenlabs.com](https://landenlabs.com)


## Introduction

Command line utility to list or find open files. Default is to list all open files preceeded with the process name and id. 
Optionally you can pass a process id to limit output to just that process
or pass part of a file name to isolate individual files. 

Code is all C++ with MS Visual Studio solution.

## Help Banner:
<pre>
List open files
llopenfiles [pid | partOfFilename]

</pre>

### Code based off of work from these github projects
 
- https://github.com/AlSch092/DetectOpenHandles
- https://github.com/yihleego/handle-tools/tree/master/src

### Win32 API

- [NtQuerySystemInformation function](https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntquerysysteminformation)
- [NtQueryObject function](https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntqueryobject)
- [ZwDuplicateObject function](https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-zwduplicateobject)

### Process Utilities

- [Handle](https://docs.microsoft.com/en-us/sysinternals/downloads/handle)
- [Process Explorer](https://docs.microsoft.com/en-us/sysinternals/downloads/process-explorer)
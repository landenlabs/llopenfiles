<table border="0">
  <tr>
    <td>
      <!-- VERSION -->v6.05.24<br>
      <!-- DATE -->24-May-2026<br>
      Windows<br>
      <a href="https://landenlabs.com">Home</a>
    </td>
    <td>
      <a href="https://landenlabs.com">
        <img src="screens/landen_labs_300.webp" width="300" alt="LanDen Labs">
      </a>
    </td>
  </tr>
</table>

# llopenfiles
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
llopenfiles.exe -?

List open files v1.6 Jan 25 2025
llopenfiles
  -pid=<pid>   ; Limit scan to this pid
  -closeHandle ; When matching open handle found, try and close it
  -terminate   ; When matching open handle found, try and terminate process
  -verbose     ; Show extra information

  partOfFileName ...

Examples:
  llopenfiles                              ; Show all open files
  llopenfiles file1.txt file2.txt          ; Only show open file matches
  llopenfiles -pid=123 -pid=345            ; Show files of matched process ids
  llopenfiles -pid=123 -pid=345 filepat1 filepat2
  llopenfiles -close my_text_file.txt      ; If matched, try and close it
  llopenfiles -terminate my_text_file.txt  ; If matched, try and terminate process
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

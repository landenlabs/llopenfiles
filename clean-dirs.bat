@echo off
setlocal EnableExtensions EnableDelayedExpansion

::
:: clean up project directories
::

if "%1" == "" (
    echo Clean directories
	echo  clean-dirs  [show or clean]
    goto done
)

if "%1" == "show" (
    echo --- show directories and files to clean ---
	lr -n -P=.*\\build\\.* -r . 
	lr -n -P=.*\\x64\\.* -r . 
    lr -n -P=.*\\.vs\\.* -r .  
	lr -n -F=*.exe -r . 
	lr -n -F=*.obj -r . 
)
if "%1" == "clean" (
	echo === show directories and files to DELETE ===
	lr -q -P=.*\\build\\.* -r . 
	lr -q -P=.*\\x64\\.* -r . 
	lr -f -P=.*\\.vs\\.* -r . 
	lr -q -F=*.exe -r . 
	lr -q -F=*.obj -r .
)

:done
endlocal
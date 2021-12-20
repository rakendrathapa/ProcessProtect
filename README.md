# ProcessProtect
This sample is for using object callbacks to protect certain processes from termination by denying the `PROCESS_TERMINATE` access mask from any clent that requests it for these *Protected* processes.

The driver keeps the list of protected processes. We'll implement a simple limited array to hold the process IDs under the driver protection.

The driver also exposes 3 IO control codes to allow adding and removing PIDs as well as clearing the entire list. These control codes are defined in *ProcessProtectCommon.h*

#### Driver  Name(SymLink):  "ProcessProtect"
+ IOCTL_PROCESS_PROTECT_BY_PID  - Add the process ID to protect from deletion.
+ IOCTL_PROCESS_UNPROTECT_BY_PID - Remove the process ID which is protected from deletion.
+ IOCTL_PROCESS_PROTECT_CLEAR - Remove all processes from the protected list.

The client application should be able to add, remove, and clear processes by issuing correct DeviceIoControl calls. 

#### Test:
1. Launch Notepad.
2. Protect It.
3. Open Task Manager.
4. Try to kill it from Task Manager. This should deny the operation.
5. Remove the protection and try step 4 again. This time the process is terminated as expected.

#### Note: 
In the case of notepad, even with protection, clicking the window close button or selecting File/Exit from the menu would terminate the process. This is because it's being done
internally by calling `ExitProcess` which does not involve any handles. This means the protection mechanism we devised here is essentially good for processes without user interface.

#### TODO:
1. Add IOCTL_PROCESS_QUERY_PIDS - Allows querying the currently protected processes.
2. Donot allow thread injection into other processes unless the target process is being debugged.
3. Add this functionality into ProcessMonitorUtility Project for a unified view.


#include "pch.h"

#include "ProcessProtectCommon.h"
#include "ProcessProtect.h"
#include "AutoLock.h"

// PROTOTYPES
DRIVER_UNLOAD	ProcessProtectUnload;
DRIVER_DISPATCH	ProcessProtectCreateClose, ProcessProtectDeviceControl;

OB_PREOP_CALLBACK_STATUS OnPreOpenProcess(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION Info);

bool FindProcess(ULONG pid);
bool AddProcess(ULONG pid);
bool RemoveProcess(ULONG pid);

// Globals
Globals g_Globals;		// Mutex.

extern "C" NTSTATUS
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING Registry)
{
	UNREFERENCED_PARAMETER(Registry);
	auto status = STATUS_SUCCESS;

	KdPrint((DRIVER_PREFIX "DriverEntry entered\n"));

	g_Globals.Init();

	// Registration to object callbacks for process. Structures for registration
	OB_OPERATION_REGISTRATION operations[] =
	{
		// Registration for process object only, with pre-callback provided. This callback should remove the PROCESS_TERMINATE 
		// from the desired access requested by the client.
		{
			PsProcessType,	// Object Type
			OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE,
			OnPreOpenProcess, nullptr	// pre, post. 
		}
	};

	OB_CALLBACK_REGISTRATION reg =
	{
		OB_FLT_REGISTRATION_VERSION,
		1,	// operation count
		RTL_CONSTANT_STRING(L"12345.6171"),	// altitude
		nullptr,	// context
		operations
	};

	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\" PROCESS_PROTECT_NAME);
	UNICODE_STRING symName = RTL_CONSTANT_STRING(L"\\??\\" PROCESS_PROTECT_NAME);
	PDEVICE_OBJECT deviceObject = nullptr;

	do 
	{
		status = ObRegisterCallbacks(&reg, &g_Globals.RegHandle);
		if (!NT_SUCCESS(status))
		{
			KdPrint((DRIVER_PREFIX "failed to register callbacks (status=0x%08X)\n", status));
			break;
		}

		status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);
		if (!NT_SUCCESS(status))
		{
			KdPrint((DRIVER_PREFIX "failed to create device object (status=0x%08X)\n", status));
			break;
		}
		
		status = IoCreateSymbolicLink(&symName, &deviceName);
		if (!NT_SUCCESS(status))
		{
			KdPrint((DRIVER_PREFIX "failed to create symbolic link (status=0x%08X)\n", status));
			break;
		}
	} while (false);

	if (!NT_SUCCESS(status))
	{
		if (deviceObject)
		{
			IoDeleteDevice(deviceObject);
		}
		if (g_Globals.RegHandle)
		{
			ObUnRegisterCallbacks(&g_Globals.RegHandle);
		}
	}

	DriverObject->DriverUnload = ProcessProtectUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverObject->MajorFunction[IRP_MJ_CLOSE] = ProcessProtectCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcessProtectDeviceControl;

	KdPrint((DRIVER_PREFIX "DriverEntry completed successfully\n"));

	return status;
}

NTSTATUS ProcessProtectDeviceControl(PDEVICE_OBJECT deviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto status = STATUS_SUCCESS;
	auto len = 0;

	switch (stack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_PROCESS_PROTECT_BY_PID:
		//
		break;
	case IOCTL_PROCESS_UNPROTECT_BY_PID:
		//
		break;
	case IOCTL_PROCESS_PROTECT_CLEAR:
		//
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	// Complete the request
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

// To help with add and removing PIDS, we'll create 2 helper functions.
// Caller must acquire fast mutex before calling AddProcess and RemoveProcess
bool AddProcess(ULONG pid)
{
	for (int i = 0; i < MaxPids; i++)
	{
		if (g_Globals.Pids[i] == 0)
		{
			g_Globals.Pids[i] = pid;
			g_Globals.PidsCount++;
			return true;
		}
	}
	return false;
}

bool RemoveProcess(ULONG pid)
{
	for (int i = 0; i < MaxPids; i++)
	{
		if (g_Globals.Pids[i] == pid)
		{
			g_Globals.Pids[i] = 0;
			g_Globals.PidsCount--;
			return true;
		}
	}
	return false;
}

bool FindProcess(ULONG pid)
{
	for (int i = 0; i < MaxPids; i++)
	{
		if (g_Globals.Pids[i] == pid)
		{
			return true;
		}
	}
	return false;
}
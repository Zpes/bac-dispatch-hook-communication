#include "utils.h"

PVOID utility::get_kernel_module_by_name(char* moduleName)
{
	ULONG pool_size = 0;
	NTSTATUS status = ZwQuerySystemInformation(0xB, nullptr, 0, &pool_size);

	if (status != STATUS_INFO_LENGTH_MISMATCH)
		return 0;

	PSYSTEM_MODULE_INFORMATION system_module_info = (PSYSTEM_MODULE_INFORMATION)ExAllocatePool(NonPagedPool, pool_size);

	if (!system_module_info)
		return 0;

	status = ZwQuerySystemInformation(0xB, system_module_info, pool_size, nullptr);

	if (!NT_SUCCESS(status))
		ExFreePool(system_module_info);

	PVOID address = 0;

	for (unsigned int i = 0; i < system_module_info->NumberOfModules; i++)
	{
		SYSTEM_MODULE module_entry = system_module_info->Modules[i];

		if (strstr((char*)module_entry.FullPathName, moduleName))
			address = module_entry.ImageBase;
	}

	ExFreePool(system_module_info);
	return address;
}

// https://github.com/not-wlan/driver-hijack modified it a bit
NTSTATUS utility::find_driver_object(PDRIVER_OBJECT* DriverObject, PUNICODE_STRING DriverName)
{
	HANDLE handle{};
	OBJECT_ATTRIBUTES attributes{};
	UNICODE_STRING directory_name{};
	PVOID directory{};
	BOOLEAN success = FALSE;

	RtlInitUnicodeString(&directory_name, L"\\Driver");
	InitializeObjectAttributes(&attributes, &directory_name, OBJ_CASE_INSENSITIVE, NULL, NULL);

	NTSTATUS status = ZwOpenDirectoryObject(&handle, DIRECTORY_ALL_ACCESS, &attributes);

	if (!NT_SUCCESS(status))
		return status;

	status = ObReferenceObjectByHandle(handle, DIRECTORY_ALL_ACCESS, nullptr, KernelMode, &directory, nullptr);

	if (!NT_SUCCESS(status))
	{
		ZwClose(handle);
		return status;
	}

	POBJECT_DIRECTORY directory_object = POBJECT_DIRECTORY(directory);

	ExAcquirePushLockExclusiveEx(&directory_object->Lock, 0);

	for (POBJECT_DIRECTORY_ENTRY entry : directory_object->HashBuckets)
	{
		if (!entry)
			continue;

		if (success)
			break;

		while (entry && entry->Object)
		{
			PDRIVER_OBJECT driver = PDRIVER_OBJECT(entry->Object);

			if (RtlCompareUnicodeString(&driver->DriverName, DriverName, FALSE) == 0)
			{
				*DriverObject = driver;
				success = TRUE;
			}
			entry = entry->ChainLink;
		}
	}

	ExReleasePushLockExclusiveEx(&directory_object->Lock, 0);

	ObDereferenceObject(directory);
	ZwClose(handle);

	return success == TRUE ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

NTSTATUS utility::memory::read_virtual_memory(PKERNEL_MEMORY_REQUEST memory_request)
{
	PEPROCESS process;
	SIZE_T bytes;

	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)memory_request->pid, &process);

	if (!NT_SUCCESS(status))
		return status;

	status = MmCopyVirtualMemory(process, memory_request->source, PsGetCurrentProcess(), memory_request->buffer, memory_request->size, UserMode, &bytes);
	ObDereferenceObject(process);

	return status;
}

NTSTATUS utility::memory::write_virtual_memory(PKERNEL_MEMORY_REQUEST memory_request)
{
	PEPROCESS process;
	SIZE_T bytes;

	NTSTATUS status = PsLookupProcessByProcessId((HANDLE)memory_request->pid, &process);

	if (!NT_SUCCESS(status))
		return status;

	status = MmCopyVirtualMemory(PsGetCurrentProcess(), memory_request->buffer, process, memory_request->source, memory_request->size, UserMode, &bytes);
	ObDereferenceObject(process);

	return status;
}

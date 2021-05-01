#pragma once
#include "defines.h"

namespace utility
{
	PVOID get_kernel_module_by_name(char* moduleName);
	NTSTATUS find_driver_object(PDRIVER_OBJECT* DriverObject, PUNICODE_STRING DriverName);

	namespace memory
	{
		NTSTATUS read_virtual_memory(PKERNEL_MEMORY_REQUEST memory_request);
		NTSTATUS write_virtual_memory(PKERNEL_MEMORY_REQUEST memory_request);
	}
}


#pragma once
#include <windows.h>
#include <iostream>
#include <TlHelp32.h>

#include "defines.h"

namespace driver
{
	class kernel_interface
	{
	public:
		HANDLE driver_handle;

		kernel_interface(LPCSTR registry_path)
		{
			driver_handle = CreateFile(registry_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		}

		bool restore_original_drivercontrol()
		{
			if (driver_handle == INVALID_HANDLE_VALUE)
				return false;

			DWORD bytes;
			bool returned = false;

			if (DeviceIoControl(driver_handle, IO_RESTORE_ORIGINAL_DRIVERCONTROL, &returned, sizeof(returned), &returned, sizeof(returned), &bytes, NULL))
				return returned;
		}

		template <typename T>
		T read_virtual_memory(ULONG pid, ULONG source)
		{
			if (driver_handle == INVALID_HANDLE_VALUE)
			{
				return 0;
			}

			T buff;
			KERNEL_MEMORY_REQUEST kernel_memory_request;

			kernel_memory_request.pid = pid;
			kernel_memory_request.source = reinterpret_cast<void*>(source);
			kernel_memory_request.buffer = &buff;
			kernel_memory_request.size = sizeof(T);

			if (DeviceIoControl(driver_handle, IO_READ_VIRTUAL_MEMORY, &kernel_memory_request, sizeof(kernel_memory_request), &kernel_memory_request, sizeof(kernel_memory_request), 0, 0))
			{
				return buff;
			}

			return buff;
		}

		template <typename T>
		bool write_virtual_memory(ULONG pid, ULONG source, T buffer)
		{
			if (driver_handle == INVALID_HANDLE_VALUE)
			{
				return false;
			}

			DWORD bytes;

			KERNEL_MEMORY_REQUEST kernel_memory_request;

			kernel_memory_request.pid = pid;
			kernel_memory_request.source = reinterpret_cast<void*>(source);
			kernel_memory_request.buffer = &buffer;
			kernel_memory_request.size = sizeof(T);

			if (DeviceIoControl(driver_handle, IO_WRITE_VIRTUAL_MEMORY, &kernel_memory_request, sizeof(kernel_memory_request), 0, 0, &bytes, NULL))
			{
				return true;
			}

			return false;
		}

		ULONG get_process_id(std::string process_name)
		{
			PROCESSENTRY32 processentry;
			HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

			if (snapshot_handle == INVALID_HANDLE_VALUE)
				return NULL;

			processentry.dwSize = sizeof(MODULEENTRY32);

			while (Process32Next(snapshot_handle, &processentry))
			{
				if (process_name.compare(processentry.szExeFile) == NULL)
				{
					CloseHandle(snapshot_handle);
					return processentry.th32ProcessID;
				}
			}

			CloseHandle(snapshot_handle);
			return NULL;
		}
	};
}
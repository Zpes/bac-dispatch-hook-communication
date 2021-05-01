#include "driver.h"

PDRIVER_OBJECT driver_object = 0;
PDRIVER_DISPATCH original_dispatch = 0;

NTSTATUS init_bac_hook()
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID bac_base = utility::get_kernel_module_by_name("BadlionAnticheat");

	if (!bac_base)
	{
		DbgPrintEx(0, 0, "[INIT] couldn't find BadlionAntiCheat.sys");
		return STATUS_UNSUCCESSFUL;
	}

	DbgPrintEx(0, 0, "[INIT] BadlionAntiCheat base address -> %p", bac_base);

	UNICODE_STRING driver_name{};
	RtlInitUnicodeString(&driver_name, L"\\Driver\\BadlionAnticheat");

	status = utility::find_driver_object(&driver_object, &driver_name);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "[INIT] couldnt find driver object, error -> %lx", status);
		return STATUS_UNSUCCESSFUL;
	}

	DbgPrintEx(0, 0, "[INIT] driver name -> %wZ", &driver_name);
	DbgPrintEx(0, 0, "[INIT] driver object address -> %p", driver_object);

	original_dispatch = driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL];
	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = (PDRIVER_DISPATCH)communication::hooked_device_control;

	return STATUS_SUCCESS;
}

NTSTATUS driver_entry()
{
	DbgPrintEx(0, 0, "[DRIVER] driver loaded");

	if (NT_SUCCESS(init_bac_hook()))
		DbgPrintEx(0, 0, "[DRIVER] hooked driver control");
	else
		DbgPrintEx(0, 0, "[DRIVER] couldnt hook control");

	return STATUS_SUCCESS;
}
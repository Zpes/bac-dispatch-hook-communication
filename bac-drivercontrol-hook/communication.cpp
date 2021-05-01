#include "communicaton.h"

NTSTATUS communication::hooked_device_control(PDEVICE_OBJECT pDevice, PIRP Irp)
{
	ULONG ByteIO = 0;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	if (!NT_SUCCESS(is_ctl_valid(stack->Parameters.DeviceIoControl.IoControlCode)))
	{
		original_dispatch(pDevice, Irp);
		return 0;
	}

	DbgPrintEx(0, 0, "[HOOK] IoControlCode (CTL_CODE) -> %lx", stack->Parameters.DeviceIoControl.IoControlCode);

	switch (stack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IO_RESTORE_ORIGINAL_DRIVERCONTROL:
		{
			if (NT_SUCCESS(restore_original_device_control(original_dispatch)))
			{
				PBOOLEAN request_return = (PBOOLEAN)Irp->AssociatedIrp.SystemBuffer;
				*request_return = TRUE;
				ByteIO = sizeof(request_return);
				status = STATUS_SUCCESS;
				break;
			}
		}

	case IO_READ_VIRTUAL_MEMORY:
		{
			PKERNEL_MEMORY_REQUEST memory_request = (PKERNEL_MEMORY_REQUEST)Irp->AssociatedIrp.SystemBuffer;
			utility::memory::read_virtual_memory(memory_request);
			break;
		}
	
	case IO_WRITE_VIRTUAL_MEMORY:
		{
			PKERNEL_MEMORY_REQUEST memory_request = (PKERNEL_MEMORY_REQUEST)Irp->AssociatedIrp.SystemBuffer;
			utility::memory::write_virtual_memory(memory_request);
			break;
		}
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = ByteIO;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS communication::restore_original_device_control(PDRIVER_DISPATCH original_dispatch_param)
{
	if (!original_dispatch_param)
		return STATUS_UNSUCCESSFUL;

	driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = (PDRIVER_DISPATCH)original_dispatch_param;

	DbgPrintEx(0, 0, "[HOOK] original device control restored");
	return STATUS_SUCCESS;
}

// if you are going to add new CTL codes you need to add them to this switch. this is for filtering out actual BLC calls and our calls
NTSTATUS communication::is_ctl_valid(ULONG ctl_code)
{
	switch (ctl_code)
	{
	case IO_RESTORE_ORIGINAL_DRIVERCONTROL:
		return STATUS_SUCCESS;
		break;
	case IO_WRITE_VIRTUAL_MEMORY:
		return STATUS_SUCCESS;
		break;
	case IO_READ_VIRTUAL_MEMORY:
		return STATUS_SUCCESS;
		break;
	}

	return STATUS_UNSUCCESSFUL;
}

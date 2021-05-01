#pragma once
#include "utils.h"
#include "data.h"

namespace communication
{
	NTSTATUS hooked_device_control(PDEVICE_OBJECT pDevice, PIRP Irp);
	NTSTATUS restore_original_device_control(PDRIVER_DISPATCH original_dispatch_param);
	NTSTATUS is_ctl_valid(ULONG ctl_code);
}

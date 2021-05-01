#include "communication.hpp"

BOOLEAN WINAPI main()
{
	driver::kernel_interface driver = driver::kernel_interface("\\\\.\\BadlionAnticheat");
	bool running = true;

	if (driver.driver_handle)
		printf("[INIT] successfully created handle to BadlionAntiCheat -> %p\n", driver.driver_handle);

	ULONG pid = driver.get_process_id("javaw.exe");
	if (!pid)
	{
		std::cout << "[UM] couldnt find pid" << std::endl;
		running = false;
	}

	while (running && !GetAsyncKeyState(VK_END))
	{
		int ammo = driver.read_virtual_memory<int>(pid, 0x5EC618);
		std::cout << "[GAME] ammo -> " << ammo << std::endl;

		Sleep(1000);
	}

	if (driver.restore_original_drivercontrol())
		std::cout << "[UM] succsesfully restored original drivercontrol" << std::endl;

	if (driver.driver_handle)
		CloseHandle(driver.driver_handle);

	std::cout << "[UM] closed handle to driver" << std::endl;

	while (true);
}

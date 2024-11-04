#include "gpu.h"
#include "logging.h"

uint32_t gp0;
uint32_t gp1;
uint32_t gpu_read = 0;
//uint32_t gpu_stat = 0x10000000;
uint32_t gpu_stat = 0xFFFFFFFF;

uint32_t read_gpu(uint32_t address)
{
	if (address == 0x1F801810)
		return gpu_read;

	if (address == 0x1F801814)
		return gpu_stat;

	//log_warning("Unhandled GPU register read at address %x\n", address);

	return 0xFFFFFFFF;
}

void write_gpu(uint32_t address, uint32_t value)
{
	//log_warning("Unhandled GPU register write at address %x with value %x\n", address, value);
}

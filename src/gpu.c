#include "gpu.h"
#include "logging.h"

uint32_t read_gpu(uint32_t address)
{
	log_warning("Unhandled GPU register read at address %x\n", address);

	return 0xFFFFFFFF;
}

void write_gpu(uint32_t address, uint32_t value)
{
	log_warning("Unhandled GPU register write at address %x with value %x\n", address, value);
}

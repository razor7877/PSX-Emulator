#include "gpu.h"
#include "logging.h"

uint32_t read_gpu(uint32_t address)
{
	log_warning("Unhandled GPU register read\n");

	return 0xFFFFFFFF;
}

void write_gpu(uint32_t address, uint32_t value)
{
	log_warning("Unhandled GPU register write\n");
}

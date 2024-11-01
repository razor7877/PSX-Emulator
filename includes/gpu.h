#pragma once

#include <stdint.h>

uint32_t read_gpu(uint32_t address);
void write_gpu(uint32_t address, uint32_t value);

#pragma once

#include <stdint.h>

#define MEM_CTRL1_START 0x1F801000
#define MEM_CTRL1_END (0x1F801020 + 4)

#define PERIPHERAL_IO_START 0x1F801040
#define PERIPHERAL_IO_END (0x1F80105E + 2)

#define MEM_CTRL2_START 0x1F801060
#define MEM_CTRL2_END (0x1F801060 + 4 * 2)

#define INTERRUPT_CTRL_START 0x1F801070
#define INTERRUPT_CTRL_END (0x1F801074 + 2)

#define DMA_REGS_START 0x1F801080
#define DMA_REGS_END 0x1F8010FC

#define TIMERS_REGS_START 0x1F801100
#define TIMERS_REGS_END (0x1F801128 + 4)

#define CDROM_REGS_START 0x1F801800
#define CDROM_REGS_END (0x1F801803 + 1)

#define GPU_REGS_START 0x1F801810
#define GPU_REGS_END (0x1F801814 + 4)

#define MDEC_REGS_START 0x1F801820
#define MDEC_REGS_END (0x1F801824 + 4)

#define SPU_VOICE_START 0x1F801C00
#define SPU_VOICE_END (0x1F801C0E + 24 * 0x10)

#define SPU_CTRL_REGS_START 0x1F801D80
#define SPU_CTRL_REGS_END (0x1F801DBC + 4)

#define SPU_REVERB_START 0x1F801DC0
#define SPU_REVERB_END (0x1F801DFC + 4)

#define SPU_INTERNAL_START 0x1F801E00
#define SPU_INTERNAL_END (0x1F801E80 + 0x180)

/// <summary>
/// Functions for dispatching I/O read/write operations to the corresponding modules
/// </summary>

uint32_t read_io(uint32_t address);
void write_io(uint32_t address, uint32_t value);

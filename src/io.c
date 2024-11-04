#include "io.h"
#include "cpu.h"
#include "gpu.h"
#include "logging.h"
#include "interrupt.h"

/// <summary>
/// Reads a word from an IO port
/// </summary>
/// <param name="address">The IO port address to be read</param>
/// <returns>The word at the IO port address</returns>
uint32_t read_io(uint32_t address)
{
	if (address >= MEM_CTRL1_START && address < MEM_CTRL1_END)
	{
		log_warning("Unhandled memory control 1 read at address %x\n", address);
		return 0xFFFFFFFF;
	}

	if (address >= PERIPHERAL_IO_START && address < PERIPHERAL_IO_END)
	{
		log_warning("Unhandled peripheral IO read at address %x\n", address);
		return 0xFFFFFFFF;
	}

	if (address >= MEM_CTRL2_START && address < MEM_CTRL2_END)
	{
		log_warning("Unhandled memory control 2 read at address %x\n", address);
		return 0xFFFFFFFF;
	}

	if (address >= INTERRUPT_CTRL_START && address < INTERRUPT_CTRL_END)
	{
		return read_interrupt_control(address);
	}

	if (address >= DMA_REGS_START && address < DMA_REGS_END)
	{
		log_warning("Unhandled DMA registers read at address %x\n", address);
		return 0;
	}

	if (address >= TIMERS_REGS_START && address < TIMERS_REGS_END)
	{
		log_warning("Unhandled DMA registers read at address %x\n", address);
		return 0xFFFFFFFF;
	}

	if (address >= CDROM_REGS_START && address < CDROM_REGS_END)
	{
		log_warning("Unhandled CDROM registers read at address %x\n", address);
		return 0xFFFFFFFF;
	}

	if (address >= GPU_REGS_START && address < GPU_REGS_END)
		return read_gpu(address);

	if (address >= MDEC_REGS_START && address < MDEC_REGS_END)
	{
		log_warning("Unhandled MDEC registers read at address %x\n", address);
		return 0xFFFFFFFF;
	}

	if (address >= SPU_VOICE_START && address < SPU_VOICE_END)
	{
		log_warning("Unhandled SPU voice read at address %x\n", address);
		return 0xFFFFFFFF;
	}

	if (address >= SPU_CTRL_REGS_START && address < SPU_CTRL_REGS_END)
	{
		log_warning("Unhandled SPU control registers read at address %x\n", address);
		return 0;
	}

	if (address >= SPU_REVERB_START && address < SPU_REVERB_END)
	{
		log_warning("Unhandled SPU reverb read at address %x\n", address);
		return 0xFFFFFFFF;
	}

	if (address >= SPU_INTERNAL_START && address < SPU_INTERNAL_END)
	{
		log_warning("Unhandled SPU internal read at address %x\n", address);
		return 0xFFFFFFFF;
	}

	log_warning("Unhandled IO read at address %x\n", address);

	return 0x0;
}

void write_io(uint32_t address, uint32_t value)
{
	if (address >= MEM_CTRL1_START && address < MEM_CTRL1_END)
	{
		log_warning("Unhandled memory control 1 write at address %x\n", address);
	}
	else if (address >= PERIPHERAL_IO_START && address < PERIPHERAL_IO_END)
	{
		log_warning("Unhandled peripheral IO write at address %x\n", address);
	}
	else if (address >= MEM_CTRL2_START && address < MEM_CTRL2_END)
	{
		log_warning("Unhandled memory control 2 write at address %x\n", address);
	}
	else if (address >= INTERRUPT_CTRL_START && address < INTERRUPT_CTRL_END)
	{
		write_interrupt_control(address, value);
	}
	else if (address >= DMA_REGS_START && address < DMA_REGS_END)
	{
		log_warning("Unhandled DMA registers write at address %x\n", address);
	}
	else if (address >= TIMERS_REGS_START && address < TIMERS_REGS_END)
	{
		log_warning("Unhandled DMA registers write at address %x\n", address);
	}
	else if (address >= CDROM_REGS_START && address < CDROM_REGS_END)
	{
		log_warning("Unhandled CDROM registers write at address %x\n", address);
	}
	else if (address >= GPU_REGS_START && address < GPU_REGS_END)
	{
		write_gpu(address, value);
	}
	else if (address >= MDEC_REGS_START && address < MDEC_REGS_END)
	{
		log_warning("Unhandled MDEC registers write at address %x\n", address);
	}
	else if (address >= SPU_VOICE_START && address < SPU_VOICE_END)
	{
		log_warning("Unhandled SPU voice write at address %x\n", address);
	}
	else if (address >= SPU_CTRL_REGS_START && address < SPU_CTRL_REGS_END)
	{
		log_warning("Unhandled SPU control registers write at address %x\n", address);
	}
	else if (address >= SPU_REVERB_START && address < SPU_REVERB_END)
	{
		log_warning("Unhandled SPU reverb write at address %x\n", address);
	}
	else if (address >= SPU_INTERNAL_START && address < SPU_INTERNAL_END)
	{
		log_warning("Unhandled SPU internal write at address %x\n", address);
	}
	else
		log_warning("Unhandled IO write at address %x\n", address);
}

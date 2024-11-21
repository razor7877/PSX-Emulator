#pragma once

#include <stdint.h>

typedef enum
{
	IRQ_VBLANK = 0,
	IRQ_GPU = 1,
	IRQ_CDROM = 2,
	IRQ_DMA = 3,
	IRQ_TMR0 = 4,
	IRQ_TMR1 = 5,
	IRQ_TMR2 = 6,
	IRQ_MEM_CARD = 7, // Controller and memory card IRQ
	IRQ_SIO = 8,
	IRQ_SPU = 9,
	IRQ_CONTROLLER = 10,
} IRQ_MASK;

typedef struct
{
	uint32_t I_STAT;
	uint32_t I_MASK;
} InterruptState;

/// <summary>
/// Functions and state for emulating the PSX interrupt behavior
/// </summary>

/// <summary>
/// Reads from an interrupt related register and returns the contents
/// </summary>
/// <param name="address">The address of the interrupt register to read</param>
/// <returns>The contents of the register</returns>
uint32_t read_interrupt_control(uint32_t address);

/// <summary>
/// Writes to an interrupt related register
/// </summary>
/// <param name="address">The address of the interrupt register to write to</param>
/// <param name="value">The value to write to the register</param>
void write_interrupt_control(uint32_t address, uint32_t value);

/// <summary>
/// Triggers an IRQ that may be serviced later
/// </summary>
/// <param name="irq">The IRQ to request</param>
void request_interrupt(IRQ_MASK irq);

/// <summary>
/// Services interrupts depending on the values in I_STAT & I_MASK
/// </summary>
void service_interrupts();

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CD_FIFO_SIZE 16

/// <summary>
/// Functions and state for emulating the CD-ROM drive
/// </summary>

typedef struct
{
	int index;
	uint8_t data[CD_FIFO_SIZE];
} FIFO;

typedef struct
{
	/// <summary>
	/// The state of the index/status register
	/// </summary>
	uint8_t status_register;

	/// <summary>
	/// The current CDROM register index
	/// </summary>
	uint8_t current_index;

	/// <summary>
	/// The state of the interrupt enable register
	/// </summary>
	uint8_t interrupt_enable;

	/// <summary>
	/// The state of the interrupt flag register
	/// </summary>
	uint8_t interrupt_flag;

	FIFO parameter_fifo;
	FIFO response_fifo;
	FIFO data_fifo;

	/// <summary>
	/// Whether a CDROM IRQ is pending
	/// </summary>
	bool pending_cdrom_irq;

	/// <summary>
	/// If a CDROM IRQ is pending, how many cycles are left before triggering an IRQ
	/// </summary>
	int cycles_until_irq;
} CDController;

void reset_cdrom_state();

uint32_t read_cdrom(uint32_t address);
void write_cdrom(uint32_t address, uint32_t value);

void tick_cdrom(int cycles);

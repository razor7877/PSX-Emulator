#pragma once

#include <stdint.h>
#include <stdbool.h>

/// <summary>
/// The possible DMA transfer modes
/// </summary>
typedef enum
{
	DMA_TRANSFER_BURST = 0,
	DMA_TRANSFER_SLICE = 1,
	DMA_TRANSFER_LINKED_LIST = 2,
	DMA_TRANSFER_RESERVED = 3,
} DMATransferMode;

typedef enum
{
	DMA_DEVICE_TO_RAM = 0,
	DMA_RAM_TO_DEVICE = 1,
} DMATransferDirection;

/// <summary>
/// Represents the state of a DMA channel (CHCR settings)
/// </summary>
typedef struct
{
	DMATransferDirection dma_direction;
	bool madr_increment;
	bool bit_8;
	DMATransferMode transfer_mode;
	uint8_t chopping_dma_size;
	uint8_t chopping_cpu_size;
	bool start_transfer;
} DMATransferState;

/// <summary>
/// Represents one DMA channel and the associated state
/// </summary>
typedef struct
{
	uint32_t dma_madr; // DMA Base address
	uint32_t dma_bcr; // DMA Block control
	uint32_t dma_chcr; // DMA Channel control
	DMATransferState transfer_state;
} DMAChannel;

/// <summary>
/// Represents the entire DMA state and registers
/// </summary>
typedef struct
{
	DMAChannel channels[7];
	uint32_t dpcr;
	uint32_t dicr;
} DMA;

uint32_t read_dma_regs(uint32_t address);
void write_dma_regs(uint32_t address, uint32_t value);

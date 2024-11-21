#pragma once

#define cop0_code(value) ((((value & 0x03E00000) >> 21)))
#define CPR0(value) _cop0_registers[value]

#define BDA CPR0(5)
#define JUMPDEST CPR0(6)
#define DCIC CPR0(7)
#define BAD_VADDR CPR0(8)
#define BDAM CPR0(9)
#define BPCM CPR0(11)
#define SR CPR0(12)
#define CAUSE CPR0(13)
#define EPC CPR0(14)
#define PRID CPR0(15)

/// <summary>
/// Functions and state for emulating the coprocessor 0 and emulating exception behavior
/// </summary>

typedef enum
{
	INTERRUPT = 0,
	TLB_MODIFICATION = 1,
	TLB_LOAD = 2,
	TLB_STORE = 3,
	ADEL = 4,
	ADES = 5,
	IBE = 6,
	DBE = 7,
	SYSCALL = 8,
	BP = 9,
	RI = 0xA,
	CPU = 0xB,
	OVERFLOW = 0xC
} ExceptionType;

extern uint32_t _cop0_registers[64];

void handle_cop0_instruction();
void handle_cop1_instruction();
void handle_cop2_instruction();
void handle_cop3_instruction();

void handle_mem_exception(ExceptionType exception, uint32_t address);
void handle_exception(ExceptionType exception);

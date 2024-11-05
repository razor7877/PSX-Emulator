#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CPU_FREQ 33868800 // CPU Frequency in Hz
#define NTSC_FRAME_FREQ 59.940 // Interlaced vertical refresh rate on NTSC
#define NTSC_FRAME_CYCLE_COUNT (CPU_FREQ / NTSC_FRAME_FREQ) // How many cycles to complete one NTSC frame

#define R(reg) cpu_state.registers[reg]
#define rs(value) ((value & 0x03E00000) >> 21)
#define rt(value) ((value & 0x001F0000) >> 16)
#define rd(value) ((value & 0x0000F800) >> 11)
#define imm5(value) ((value & 0x000007C0) >> 6)

#define R0 cpu_state.registers[0]
#define R1 cpu_state.registers[1]
#define R2 cpu_state.registers[2]
#define R3 cpu_state.registers[3]
#define R4 cpu_state.registers[4]
#define R5 cpu_state.registers[5]
#define R6 cpu_state.registers[6]
#define R7 cpu_state.registers[7]
#define R8 cpu_state.registers[8]
#define R9 cpu_state.registers[9]
#define R10 cpu_state.registers[10]
#define R11 cpu_state.registers[11]
#define R12 cpu_state.registers[12]
#define R13 cpu_state.registers[13]
#define R14 cpu_state.registers[14]
#define R15 cpu_state.registers[15]
#define R16 cpu_state.registers[16]
#define R17 cpu_state.registers[17]
#define R18 cpu_state.registers[18]
#define R19 cpu_state.registers[19]
#define R20 cpu_state.registers[20]
#define R21 cpu_state.registers[21]
#define R22 cpu_state.registers[22]
#define R23 cpu_state.registers[23]
#define R24 cpu_state.registers[24]
#define R25 cpu_state.registers[25]
#define R26 cpu_state.registers[26]
#define R27 cpu_state.registers[27]
#define R28 cpu_state.registers[28]
#define R29 cpu_state.registers[29]
#define R30 cpu_state.registers[30]
#define R31 cpu_state.registers[31]

typedef struct
{
	const char* disassembly;
	void* function;
} instruction;

typedef struct
{
	// Output regs to simulate load delay slot
	uint32_t registers[32];

	/// <summary>
	/// The 32 bit value of the currently executed opcode
	/// </summary>
	uint32_t current_opcode;

	/// <summary>
	/// CPU Program counter
	/// </summary>
	uint32_t pc;

	/// <summary>
	/// Multiply/divide high result
	/// </summary>
	uint32_t hi;

	/// <summary>
	/// Multiply/divide low result
	/// </summary>
	uint32_t lo;

	/// <summary>
	/// Whether a jump should be executed on the next instruction
	/// </summary>
	bool delay_jump;

	/// <summary>
	/// The address to jump to
	/// </summary>
	uint32_t jmp_address;

	/// <summary>
	/// Instruction coutdown before loading value from memory into a register
	/// </summary>
	int delay_fetch;

	/// <summary>
	/// The register where the value should be fetched into
	/// </summary>
	int fetch_reg_index;

	/// <summary>
	/// The value that should be fetched in the register
	/// </summary>
	uint32_t fetch_reg_value;
} cpu;

extern cpu cpu_state;

/// <summary>
/// Resets the state of the CPU
/// </summary>
void reset_cpu_state();

/// <summary>
/// Triggers a load delay slot
/// </summary>
/// <param name="reg_index">The register in which the value should be fetched after the delay</param>
/// <param name="value">The value to be fetched into the register</param>
static void delay_reg_fetch(int reg_index, uint32_t value);

/// <summary>
/// Checks for putchar() calls to the TTY output
/// </summary>
static void check_tty_output();

/// <summary>
/// Prints debug info about the CPU state
/// </summary>
/// <param name="cpu_state">The state for which debug information should be printed</param>
void print_debug_info(cpu cpu_state);

/// <summary>
/// Prints the current TTY output to the console
/// </summary>
void print_tty_output();

/// <summary>
/// Handles the next CPU instruction in the emulation loop
/// </summary>
/// <param name="debug_info">Whether debug information about the instruction should be printed</param>
void handle_instruction(bool debug_info);

extern const instruction primary_opcodes[0x40];
extern const instruction secondary_opcodes[0x40];

void undefined();

// Standard opcodes
void b_cond_z();
void j();
void jal();
void beq();
void bne();
void blez();
void bgtz();
void addi();
void addiu();
void slti();
void sltiu();
void andi();
void ori();
void xori();
void lui();
void lb();
void lh();
void lwl();
void lw();
void lbu();
void lhu();
void lwr();
void sb();
void sh();
void swl();
void sw();
void swr();
void lwc0();
void lwc1();
void lwc2();
void lwc3();
void swc0();
void swc1();
void swc2();
void swc3();

// Secondary opcodes
void sll();
void srl();
void sra();
void sllv();
void srlv();
void srav();
void jr();
void jalr();
void syscall();
void op_break();
void mfhi();
void mthi();
void mflo();
void mtlo();
void mult();
void multu();
void op_div();
void divu();
void add();
void addu();
void sub();
void subu();
void and();
void op_or();
void op_xor();
void nor();
void slt();
void sltu();

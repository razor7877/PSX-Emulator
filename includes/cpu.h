#pragma once

#include <stdint.h>

#define R(reg) _registers[reg]
#define rs(value) ((value & 0x03E00000) >> 21)
#define rt(value) ((value & 0x001F0000) >> 16)
#define rd(value) ((value & 0x0000F800) >> 11)
#define imm5(value) ((value & 0x000007C0) >> 6)

#define R0 _registers[0]
#define R1 _registers[1]
#define R2 _registers[2]
#define R3 _registers[3]
#define R4 _registers[4]
#define R5 _registers[5]
#define R6 _registers[6]
#define R7 _registers[7]
#define R8 _registers[8]
#define R9 _registers[9]
#define R10 _registers[10]
#define R11 _registers[11]
#define R12 _registers[12]
#define R13 _registers[13]
#define R14 _registers[14]
#define R15 _registers[15]
#define R16 _registers[16]
#define R17 _registers[17]
#define R18 _registers[18]
#define R19 _registers[19]
#define R20 _registers[20]
#define R21 _registers[21]
#define R22 _registers[22]
#define R23 _registers[23]
#define R24 _registers[24]
#define R25 _registers[25]
#define R26 _registers[26]
#define R27 _registers[27]
#define R28 _registers[28]
#define R29 _registers[29]
#define R30 _registers[30]
#define R31 _registers[31]

typedef struct
{
	const char* disassembly;
	void* function;
} instruction;

extern uint32_t _registers[32];
extern uint32_t current_opcode;
extern uint32_t pc;

void reset_cpu_state();

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
void div();
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

void handle_instruction();

#pragma once

#include <stdint.h>

typedef struct
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2_3[2];
	uint32_t r4_7[4];
	uint32_t r8_15[8];
	uint32_t r16_23[8];
	uint32_t r24_25[2];
	uint32_t r26_27[2];
	uint32_t r28;
	uint32_t r29;
	uint32_t r30;
	uint32_t r31;
} registers;

typedef struct
{
	const char* disassembly;
	void* function;
} instruction;

void undefined();

// Standard opcodes
void special();
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
void cop0();
void cop1();
void cop2();
void cop3();
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
void or();
void xor();
void nor();
void slt();
void sltu();

void handle_instruction();

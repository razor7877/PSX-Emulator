#pragma once

#include <stdint.h>

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

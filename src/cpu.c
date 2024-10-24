#include "cpu.h"
#include "logging.h"

#define R0 _registers.r0
#define R1 _registers.r1
#define R2 _registers.r2_3[0]
#define R3 _registers.r2_3[1]
#define R4 _registers.r4_7[0]
#define R5 _registers.r4_7[1]
#define R6 _registers.r4_7[2]
#define R7 _registers.r4_7[3]
#define R8 _registers.r8_15[0]
#define R9 _registers.r8_15[1]
#define R10 _registers.r8_15[2]
#define R11 _registers.r8_15[3]
#define R12 _registers.r8_15[4]
#define R13 _registers.r8_15[5]
#define R14 _registers.r8_15[6]
#define R15 _registers.r8_15[7]
#define R16 _registers.r16_23[0]
#define R17 _registers.r16_23[1]
#define R18 _registers.r16_23[2]
#define R19 _registers.r16_23[3]
#define R20 _registers.r16_23[4]
#define R21 _registers.r16_23[5]
#define R22 _registers.r16_23[6]
#define R23 _registers.r16_23[7]
#define R24 _registers.r24_25[0]
#define R25 _registers.r24_25[1]
#define R26 _registers.r26_27[0]
#define R27 _registers.r26_27[1]
#define R28 _registers.r28
#define R29 _registers.r29
#define R30 _registers.r30
#define R31 _registers.r31

// Initialize registers
registers _registers = {
	.r0 = 0,
	.r1 = 0,
	.r2_3 = {0},
	.r4_7 = {0},
	.r8_15 = {0},
	.r16_23 = {0},
	.r24_25 = {0},
	.r26_27 = {0},
	.r28 = 0,
	.r29 = 0,
	.r30 = 0,
	.r31 = 0
};

const instruction primary_opcodes[0x40] = {
    { "SPECIAL", special },       // 00h
    { "BCONDZ", b_cond_z },       // 01h
    { "J", j },                   // 02h
    { "JAL", jal },               // 03h
    { "BEQ", beq },               // 04h
    { "BNE", bne },               // 05h
    { "BLEZ", blez },             // 06h
    { "BGTZ", bgtz },             // 07h
    { "ADDI", addi },             // 08h
    { "ADDIU", addiu },           // 09h
    { "SLTI", slti },             // 0Ah
    { "SLTIU", sltiu },           // 0Bh
    { "ANDI", andi },             // 0Ch
    { "ORI", ori },               // 0Dh
    { "XORI", xori },             // 0Eh
    { "LUI", lui },               // 0Fh
    { "COP0", cop0 },             // 10h
    { "COP1", cop1 },             // 11h
    { "COP2", cop2 },             // 12h
    { "COP3", cop3 },             // 13h
    { "N/A", undefined },         // 14h
    { "N/A", undefined },         // 15h
    { "N/A", undefined },         // 16h
    { "N/A", undefined },         // 17h
    { "N/A", undefined },         // 18h
    { "N/A", undefined },         // 19h
    { "N/A", undefined },         // 1Ah
    { "N/A", undefined },         // 1Bh
    { "N/A", undefined },         // 1Ch
    { "N/A", undefined },         // 1Dh
    { "N/A", undefined },         // 1Eh
    { "N/A", undefined },         // 1Fh
    { "LB", lb },                 // 20h
    { "LH", lh },                 // 21h
    { "LWL", lwl },               // 22h
    { "LW", lw },                 // 23h
    { "LBU", lbu },               // 24h
    { "LHU", lhu },               // 25h
    { "LWR", lwr },               // 26h
    { "N/A", undefined },         // 27h
    { "SB", sb },                 // 28h
    { "SH", sh },                 // 29h
    { "SWL", swl },               // 2Ah
    { "SW", sw },                 // 2Bh
    { "N/A", undefined },         // 2Ch
    { "N/A", undefined },         // 2Dh
    { "SWR", swr },               // 2Eh
    { "N/A", undefined },         // 2Fh
    { "LWC0", lwc0 },             // 30h
    { "LWC1", lwc1 },             // 31h
    { "LWC2", lwc2 },             // 32h
    { "LWC3", lwc3 },             // 33h
    { "N/A", undefined },         // 34h
    { "N/A", undefined },         // 35h
    { "N/A", undefined },         // 36h
    { "N/A", undefined },         // 37h
    { "SWC0", swc0 },             // 38h
    { "SWC1", swc1 },             // 39h
    { "SWC2", swc2 },             // 3Ah
    { "SWC3", swc3 },             // 3Bh
    { "N/A", undefined },         // 3Ch
    { "N/A", undefined },         // 3Dh
    { "N/A", undefined },         // 3Eh
    { "N/A", undefined }          // 3Fh
};


const instruction secondary_opcodes[0x40] = {
    { "SLL", sll },               // 00h
    { "N/A", undefined },         // 01h
    { "SRL", srl },               // 02h
    { "SRA", sra },               // 03h
    { "SLLV", sllv },             // 04h
    { "N/A", undefined },         // 05h
    { "SRLV", srlv },             // 06h
    { "SRAV", srav },             // 07h
    { "JR", jr },                 // 08h
    { "JALR", jalr },             // 09h
    { "N/A", undefined },         // 0Ah
    { "N/A", undefined },         // 0Bh
    { "SYSCALL", syscall },       // 0Ch
    { "BREAK", op_break },        // 0Dh
    { "N/A", undefined },         // 0Eh
    { "N/A", undefined },         // 0Fh
    { "MFHI", mfhi },             // 10h
    { "MTHI", mthi },             // 11h
    { "MFLO", mflo },             // 12h
    { "MTLO", mtlo },             // 13h
    { "N/A", undefined },         // 14h
    { "N/A", undefined },         // 15h
    { "N/A", undefined },         // 16h
    { "N/A", undefined },         // 17h
    { "MULT", mult },             // 18h
    { "MULTU", multu },           // 19h
    { "DIV", div },               // 1Ah
    { "DIVU", divu },             // 1Bh
    { "N/A", undefined },         // 1Ch
    { "N/A", undefined },         // 1Dh
    { "N/A", undefined },         // 1Eh
    { "N/A", undefined },         // 1Fh
    { "ADD", add },               // 20h
    { "ADDU", addu },             // 21h
    { "SUB", sub },               // 22h
    { "SUBU", subu },             // 23h
    { "AND", and },               // 24h
    { "OR", or },                 // 25h
    { "XOR", xor },               // 26h
    { "NOR", nor },               // 27h
    { "N/A", undefined },         // 28h
    { "N/A", undefined },         // 29h
    { "SLT", slt },               // 2Ah
    { "SLTU", sltu },             // 2Bh
    { "N/A", undefined },         // 2Ch
    { "N/A", undefined },         // 2Dh
    { "N/A", undefined },         // 2Eh
    { "N/A", undefined },         // 2Fh
    { "N/A", undefined },         // 30h
    { "N/A", undefined },         // 31h
    { "N/A", undefined },         // 32h
    { "N/A", undefined },         // 33h
    { "N/A", undefined },         // 34h
    { "N/A", undefined },         // 35h
    { "N/A", undefined },         // 36h
    { "N/A", undefined },         // 37h
    { "N/A", undefined },         // 38h
    { "N/A", undefined },         // 39h
    { "N/A", undefined },         // 3Ah
    { "N/A", undefined },         // 3Bh
    { "N/A", undefined },         // 3Ch
    { "N/A", undefined },         // 3Dh
    { "N/A", undefined },         // 3Eh
    { "N/A", undefined }          // 3Fh
};


/// <summary>
/// Program counter
/// </summary>
uint32_t pc = 0xBFC00000;

/// <summary>
/// Multiply/divide high result
/// </summary>
uint32_t hi = 0;

/// <summary>
/// Multiply/divde low result
/// </summary>
uint32_t lo = 0;

void undefined()
{
    log_error("Attempted to execute undefined opcode!\n");
}

void special()
{

}

void b_cond_z()
{

}

void j()
{

}

void jal()
{

}

void beq()
{

}

void bne()
{

}

void blez()
{

}

void bgtz()
{

}

void addi()
{

}

void addiu()
{

}

void slti()
{

}

void sltiu()
{

}

void andi()
{

}

void ori()
{

}

void xori()
{

}

void lui()
{

}

void cop0()
{

}

void cop1()
{

}

void cop2()
{

}

void cop3()
{

}

void lb()
{

}

void lh()
{

}

void lwl()
{

}

void lw()
{

}

void lbu()
{

}

void lhu()
{

}

void lwr()
{

}

void sb()
{

}

void sh()
{

}

void swl()
{

}

void sw()
{

}

void swr()
{

}

void lwc0()
{

}

void lwc1()
{

}

void lwc2()
{

}

void lwc3()
{

}

void swc0()
{

}

void swc1()
{

}

void swc2()
{

}

void swc3()
{

}

void sll()
{

}

void srl()
{

}

void sra()
{

}

void sllv()
{

}

void srlv()
{

}

void srav()
{

}

void jr()
{

}

void jalr()
{

}

void syscall()
{

}

void op_break()
{

}

void mfhi()
{

}

void mthi()
{

}

void mflo()
{

}

void mtlo()
{

}

void mult()
{

}

void multu()
{

}

void div()
{

}

void divu()
{

}

void add()
{

}

void addu()
{

}

void sub()
{

}

void subu()
{

}

void and()
{

}

void or()
{

}

void xor()
{

}

void nor()
{

}

void slt()
{

}

void sltu()
{

}

void handle_instruction()
{

}

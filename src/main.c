#include <stdio.h>
#include <stdint.h>

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

void handle_instruction()
{

}

int main(int argc, char** argv)
{
	printf("Startup test\n");

	return 0;
}
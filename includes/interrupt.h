#pragma once

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
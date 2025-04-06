#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "frontend/gl.h"
#include "frontend/ui.h"
#include "cpu.h"
#include "memory.h"
#include "debug.h"

UIState ui_state = {
    .ctx = NULL,
    .io = NULL
};

void gui_init()
{
    ui_state.ctx = igCreateContext(NULL);
    ui_state.io = igGetIO();
    ui_state.io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    const char* glsl_version = "#version 410 core";
    ImGui_ImplGlfw_InitForOpenGL(frontend_state.window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    igStyleColorsDark(NULL);
}

void gui_terminate()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(ui_state.ctx);
}

void gui_render()
{
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

void gui_update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    igBeginMainMenuBar();

    ImVec2 main_menu_bar_size;
    igGetWindowSize(&main_menu_bar_size);

    if (igBeginMenu("File", true))
    {
        igMenuItemEx("Load file", NULL, NULL, false, true);

        igEndMenu();
    }

    if (igBeginMenu("Run", true))
    {
        if (igMenuItemEx("Reset", NULL, NULL, false, true))
            reset_emulator();

        igEndMenu();
    }

    igEndMainMenuBar();

    // We want to create a full size window
    ImVec2 window_size = {
        frontend_state.window_size.x,
        frontend_state.window_size.y - main_menu_bar_size.y
    };

    igSetNextWindowPos(
        (struct ImVec2) { 0, main_menu_bar_size.y },
        ImGuiCond_Always,
        (struct ImVec2) { 0, 0 }
    );
    igSetNextWindowSize(window_size, ImGuiCond_Always);
    igBegin("Main Window", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);

    igDockSpace(igGetID_Str("MainDockSpace"), (struct ImVec2) { 0, 0 }, ImGuiDockNodeFlags_None, NULL);

    igBegin("PSX", NULL, ImGuiWindowFlags_None);

    ImVec2 emulator_window_size;
    igGetContentRegionAvail(&emulator_window_size);

    igImage(
        (ImTextureID)PSX_RT.render_texture,
        emulator_window_size,
        (struct ImVec2) { 0.0f, 1.0f },
        (struct ImVec2) { 1.0f, 0.0f },
        (struct ImVec4) { 1.0f, 1.0f, 1.0f, 1.0f },
        (struct ImVec4) {0}
    );

    igEnd();

    igBegin("VRAM", NULL, ImGuiWindowFlags_None);

    ImVec2 vram_window_size;
    igGetContentRegionAvail(&vram_window_size);

    igImage(
        (ImTextureID)VRAM_RT.render_texture,
        vram_window_size,
        (struct ImVec2) { 0.0f, 1.0f },
        (struct ImVec2) { 1.0f, 0.0f },
        (struct ImVec4) { 1.0f, 1.0f, 1.0f, 1.0f },
        (struct ImVec4) {0}
    );

    igEnd();

    igBegin("Disassembler", NULL, ImGuiWindowFlags_None);

    const int instruction_count = 20;

    // We show the instructions before and after the current one
    for (int i = -instruction_count / 2; i < instruction_count / 2; i++)
    {
        uint32_t opcode_address = cpu_state.pc + i * 4;
        uint32_t opcode = read_word(opcode_address);

        // Get primary opcode from 6 highest bits
        uint8_t primary_opcode = (opcode & 0xFC000000) >> 26;
        // Get secondary opcode from 6 lowest bits
        uint8_t secondary_opcode = opcode & 0x3F;

        const char* disassembly = primary_opcodes[primary_opcode].disassembly;
        if (primary_opcode == 0x00)
            disassembly = secondary_opcodes[secondary_opcode].disassembly;

        Breakpoint* breakpoint = NULL;

        // Check if there is a breakpoint for this address
        for (int i = 0; i < debug_state.breakpoint_count; i++)
        {
            Breakpoint* br = &debug_state.code_breakpoints[i];
            if (br->address == opcode_address)
                breakpoint = br;
        }

        bool pushed_color = false;

        if (i == 0) // The instruction being executed
        {
            igPushStyleColor_Vec4(ImGuiCol_Text, (struct ImVec4) { 0.5f, 0.5f, 1.0f, 1.0f });
            pushed_color = true;
        }
        else if(breakpoint && (breakpoint->break_on_code || breakpoint->break_on_data))
        {
            igPushStyleColor_Vec4(ImGuiCol_Text, (struct ImVec4) { 0.8f, 0.2f, 0.2f, 1.0f });
            pushed_color = true;
        }

        igText("%08x | %s - %08x\tRS(r%d): %x RT(r%d): %x RD(r%d): %x\n",
            opcode_address, disassembly, opcode,
            rs(opcode), R(rs(opcode)),
            rt(opcode), R(rt(opcode)),
            rd(opcode), R(rd(opcode))
        );
        
        if (pushed_color)
            igPopStyleColor(1);
    }

    if (debug_state.in_debug)
    {
        if (igButton("Resume", (struct ImVec2) { 100, 20 }))
            debug_state.in_debug = false;
    }
    else
    {
        if (igButton("Pause", (struct ImVec2) { 100, 20 }))
            debug_state.in_debug = true;
    }
    igSameLine(0, -1);

    if (!debug_state.in_debug)
    {
        igPushItemFlag(ImGuiItemFlags_Disabled, true);
        igPushStyleVar_Float(ImGuiStyleVar_Alpha, 0.5f);
    }

    if (igButton("Step over", (struct ImVec2) { 80, 20 }))
    {

    }
    igSameLine(0, -1);

    if (igButton("Step into", (struct ImVec2) { 80, 20 }))
        handle_instruction(true);

    igSameLine(0, -1);

    if (igButton("Step out", (struct ImVec2) { 80, 20 }))
    {

    }
    igSameLine(0, -1);

    if (!debug_state.in_debug)
    {
        igPopItemFlag();
        igPopStyleVar(1);
    }

    igEnd();

    igBegin("Debug", NULL, ImGuiWindowFlags_None);

    igText("Breakpoints");

    if (igButton("Add breakpoint", (struct ImVec2) { 120, 20 }))
        add_breakpoint(cpu_state.pc, false, false);

    for (int i = 0; i < MAX_BREAKPOINTS; i++)
    {
        Breakpoint* br = &debug_state.code_breakpoints[i];

        if (br->in_use)
        {
            igPushID_Ptr(br);

            igText("%02d:", i);
            igSameLine(0, -1);

            static char address_input[MAX_BREAKPOINTS][256];
            snprintf(address_input[i], sizeof(address_input[i]), "%x", br->address);

            igPushItemWidth(100);
            if (igInputText("", address_input[i], sizeof(address_input[i]), 0, 0, NULL))
            {
                uint32_t address = strtoull(address_input[i], NULL, 16);
                br->address = address;
            }
            igPopItemWidth();
            igSameLine(0, -1);

            igText("Break on:");
            igSameLine(0, -1);

            igCheckbox("Code", &br->break_on_code);
            igSameLine(0, -1);

            igCheckbox("Data", &br->break_on_data);
            igSameLine(0, -1);

            if (igButton("X", (struct ImVec2) { 20, 20 }))
                delete_breakpoint(i);

            igPopID();
        }
    }

    igEnd();

    igBegin("CPU", NULL, ImGuiWindowFlags_None);

    igPushStyleColor_Vec4(ImGuiCol_Text, (struct ImVec4) { 0.8f, 0.8f, 1.0f, 1.0f });
    igText("pc: ");
    igPopStyleColor(1);
    igSameLine(0, -1);
    igText("%08x ", cpu_state.pc);
    igSameLine(0, 30);

    igPushStyleColor_Vec4(ImGuiCol_Text, (struct ImVec4) { 0.8f, 0.8f, 1.0f, 1.0f });
    igText("hi: ");
    igPopStyleColor(1);
    igSameLine(0, -1);
    igText("%08x ", cpu_state.hi);
    igSameLine(0, 30);

    igPushStyleColor_Vec4(ImGuiCol_Text, (struct ImVec4) { 0.8f, 0.8f, 1.0f, 1.0f });
    igText("lo: ");
    igPopStyleColor(1);
    igSameLine(0, -1);
    igText("%08x", cpu_state.lo);

    for (int i = 0; i < 32; i++)
    {
        int index = 8 * (i % 4) + (i / 4);
        igPushStyleColor_Vec4(ImGuiCol_Text, (struct ImVec4) { 0.8f, 0.8f, 1.0f, 1.0f });
        igText("r%02d: ", index);
        igPopStyleColor(1);
        igSameLine(0, -1);
        igText("%08x", cpu_state.registers[index]);

        if ((i % 4) != 3)
            igSameLine(0, 30);
    }

    igEnd();

    igBegin("TTY Output", NULL, ImGuiWindowFlags_None);

    if (igButton("Clear", (struct ImVec2) { 80, 20 }))
        memset(debug_state.tty, 0, sizeof(debug_state.tty));

    igText("%s", debug_state.tty);

    igEnd();

    igEnd();
}

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include "frontend/gl.h"
#include "cpu.h"
#include "memory.h"

static ImGuiContext* ctx;
static ImGuiIO* io;

void gui_init()
{
    ctx = igCreateContext(NULL);
    io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
    igDestroyContext(ctx);
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

    if (igBeginMenu("Windows", true))
    {
        igMenuItemEx("File", NULL, NULL, false, true);

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

    for (int i = -instruction_count / 2; i < instruction_count / 2; i++)
    {
        // Get primary opcode from 6 highest bits
        uint8_t primary_opcode = (cpu_state.current_opcode & 0xFC000000) >> 26;
        // Get secondary opcode from 6 lowest bits
        uint8_t secondary_opcode = cpu_state.current_opcode & 0x3F;

        char* disassembly = primary_opcodes[primary_opcode].disassembly;
        if (primary_opcode == 0x00)
            disassembly = secondary_opcodes[secondary_opcode].disassembly;

        if (i == 0)
            igPushStyleColor_Vec4(ImGuiCol_Text, (struct ImVec4) { 0.5f, 0.5f, 1.0f, 1.0f });

        uint32_t opcode = read_word(cpu_state.pc + i);

        igText("%x : %s - %x\tRS(r%d): %x RT(r%d): %x RD(r%d): %x\n",
            cpu_state.pc + i, disassembly, opcode,
            rs(opcode), R(rs(opcode)),
            rt(opcode), R(rt(opcode)),
            rd(opcode), R(rd(opcode))
        );
        
        if (i == 0)
            igPopStyleColor(1);
    }

    igEnd();

    igBegin("Breakpoints", NULL, ImGuiWindowFlags_None);

    igEnd();

    igEnd();
}

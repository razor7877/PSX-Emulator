#include "frontend/gl.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

static ImGuiContext* ctx;
static ImGuiIO* io;

void gui_init()
{
    ctx = igCreateContext(NULL);
    io = igGetIO();

    const char* glsl_version = "#version 330 core";
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

gui_update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    igBegin("Test", NULL, 0);
    igText("Test");
    igButton("Test", (struct ImVec2) { 0, 0 });
    igEnd();

    // // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. 
    // // Here we just want to make the demo initial state a bit more friendly!
    // igSetNextWindowPos((struct ImVec2){0,0}, ImGuiCond_FirstUseEver,(struct ImVec2){0,0} ); 
    igShowDemoWindow(NULL);
}

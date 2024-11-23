#pragma once

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

/// <summary>
/// Functions and state for the ImGui interfaces
/// </summary>

typedef struct
{
	ImGuiContext* ctx;
	ImGuiIO* io;
} UIState;

void gui_init();
void gui_render();
void gui_update();
void gui_terminate();

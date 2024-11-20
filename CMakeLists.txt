cmake_minimum_required(VERSION 3.24)
project(PSX_Emulator)

# Add library subfolders
add_subdirectory(libs/glfw EXCLUDE_FROM_ALL)

# Add cimgui directories
include_directories(
	libs
	libs/cimgui
	libs/cimgui/imgui
	libs/cimgui/imgui/backends
	libs/glfw/include
)

file(GLOB cimgui_files
	"libs/cimgui/*.cpp" 
	"libs/cimgui/imgui/*.cpp"
	"libs/cimgui/imgui/backends/imgui_impl_glfw.cpp"
	"libs/cimgui/imgui/backends/imgui_impl_opengl3.cpp"
)

add_library(cimgui ${cimgui_files})

target_compile_definitions(cimgui PRIVATE
	IMGUI_IMPL_API=extern\ \"C\")

# Query source & include files
file(GLOB SRC "src/*.c")
file(GLOB SRC_FRONTEND "src/frontend/*.c")

file(GLOB INCLUDES "includes/*.h")
file(GLOB INCLUDES_FRONTEND "includes/frontend/*.h")

add_executable(PSX_Emulator
	${SRC}
	${SRC_FRONTEND}
	${INCLUDES}
	${INCLUDES_FRONTEND}
	src/glad/glad.c
)

# Group files with the same hierarchy as in the file system
source_group("src" FILES ${SRC})
source_group("src\\frontend" FILES ${SRC_FRONTEND})

source_group("includes" FILES ${INCLUDES})
source_group("includes\\frontend" FILES ${INCLUDES_FRONTEND})

# Add include folders so they are found by the compiler
target_include_directories(PSX_Emulator PUBLIC
	includes
	libs
)

target_link_libraries(PSX_Emulator PUBLIC glfw cimgui)

file(COPY roms DESTINATION ${PSX_Emulator_BINARY_DIR})

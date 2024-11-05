cmake_minimum_required(VERSION 3.24)
project(PSX_Emulator)

# Query source & include files
file(GLOB SRC "src/*.c")
file(GLOB INCLUDES "includes/*.h")

# Add library subfolders
add_subdirectory(libs/glfw EXCLUDE_FROM_ALL)

add_executable(PSX_Emulator ${SRC} ${INCLUDES} src/glad/glad.c)

# Group files with the same hierarchy as in the file system
source_group("src" FILES ${SRC})
source_group("includes" FILES ${INCLUDES})

# Add include folders so they are found by the compiler
target_include_directories(PSX_Emulator PUBLIC
	includes
	libs
)

target_link_libraries(PSX_Emulator PUBLIC glfw)

file(COPY roms DESTINATION ${PSX_Emulator_BINARY_DIR})

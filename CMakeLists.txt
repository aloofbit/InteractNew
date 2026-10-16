# Interact.dll. 32-bit only: the 1.12 client is x86, so the DLL loaded into it must be too.
#   git submodule update --init
#   cmake -B build -A Win32 && cmake --build build --config Release
cmake_minimum_required(VERSION 3.15)
# CMP0091 must be NEW before project() for CMAKE_MSVC_RUNTIME_LIBRARY to take effect.
cmake_policy(SET CMP0091 NEW)

# Static CRT: the DLL is dropped into a game folder that may have no VC++ redistributable.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(Interact C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# minhook asks for CMake 2.8, which CMake 4 refuses.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
add_subdirectory(minhook)

add_library(Interact SHARED
    dllmain.cpp
    Game.cpp
)

target_link_libraries(Interact PRIVATE minhook)

# Interact.dll, loaded by VanillaFixes from dlls.txt. The DLL lands in the project root;
# the empty $<0:> stops MSVC adding a per-config subfolder.
set_target_properties(Interact PROPERTIES
    OUTPUT_NAME "Interact"
    PREFIX ""
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}$<0:>")

if(MSVC)
    target_compile_options(Interact PRIVATE /W3 /permissive- /GS- /utf-8)
endif()

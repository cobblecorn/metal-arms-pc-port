cmake_minimum_required(VERSION 3.20)
project(metal_arms_port C CXX ASM_MASM)

# The original code base relies on 32-bit pointers (data-file fixups, inline
# x86 asm, Bink x86 import lib), so this is x86-only for now.
#   cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Metal Arms port is 32-bit only for now. Configure with -A Win32.")
endif()

# Build the runtime with the GameCube struct layout (FANG_WINGC: GC alignment,
# no SSE) so structures read from the retail GC data files match in memory.
option(MA_GC_DATA_LAYOUT "Use the GameCube struct layout (FANG_WINGC)" ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(MA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/ma)
set(MA_SHADER_DIR ${CMAKE_BINARY_DIR}/shaders)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
include(cmake/sources_fang2.cmake)
include(cmake/sources_ma.cmake)

# ---------------------------------------------------------------------------
# Common flags. The original project used: conformant for-scope, no C++
# exceptions, static-analysis-free /W3.
# ---------------------------------------------------------------------------
function(ma_common_settings tgt)
    target_compile_definitions(${tgt} PRIVATE
        WIN32 _WINDOWS _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS
        NOMINMAX
        _FANGDEF_PLATFORM_WIN
        _FANGDEF_ENABLE_DEV_FEATURES
        $<$<BOOL:${MA_GC_DATA_LAYOUT}>:_FANGDEF_WINGC>
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG _FANGDEF_TEST_BUILD>
    )
    target_compile_options(${tgt} PRIVATE
        $<$<COMPILE_LANGUAGE:C,CXX>:/W3 /MP /permissive /Zc:strictStrings- /Zc:wchar_t->
        # Release keeps symbols and frame pointers: crash logs and the -port-diag stall sampler
        # (port/main_win.cpp) name functions and walk the stack there too.
        $<$<AND:$<CONFIG:Release>,$<COMPILE_LANGUAGE:C,CXX>>:/Zi /Oy->
        $<$<COMPILE_LANGUAGE:C,CXX>:/wd4996 /wd4244 /wd4267 /wd4305 /wd4018 /wd4101 /wd4102 /wd4800 /wd4390 /wd5208 /wd4091>
    )
    target_include_directories(${tgt} PRIVATE
        ${CMAKE_SOURCE_DIR}/port/compat            # d3d8/d3dx8 shims (D3D9 backend)
        ${CMAKE_SOURCE_DIR}/port
        ${MA_ROOT}/Lib/SmallAMX
        ${MA_ROOT}/Lib/Fang2
        ${MA_ROOT}/Lib/Fang2/dx
        ${MA_ROOT}/Lib/Fang2/dx/win
        ${MA_ROOT}/Lib/Bink
        ${MA_SHADER_DIR}                          # generated CompiledVShader*/PShader*.h
    )
endfunction()

# ---------------------------------------------------------------------------
# SmallAMX: the scripting VM
# ---------------------------------------------------------------------------
add_library(smallamx STATIC
    ma/Lib/SmallAMX/amx.c
    ma/Lib/SmallAMX/amxcons.c
    ma/Lib/SmallAMX/amxcore.c)
ma_common_settings(smallamx)

# ---------------------------------------------------------------------------
# Fang2: the engine (DX backend)
# ---------------------------------------------------------------------------
# The D3D8 shader sources (.nvv/.nvp) are assembled to C headers at build time.
file(GLOB MA_SHADER_SRCS
    ${MA_ROOT}/Lib/Fang2/dx/*.nvv
    ${MA_ROOT}/Lib/Fang2/dx/*.nvp
    ${MA_ROOT}/Lib/Fang2/dx/fdxsh_psprite.vsh
    ${MA_ROOT}/Lib/Fang2/dx/fdx8vshader_const.h)
add_custom_command(
    OUTPUT ${MA_SHADER_DIR}/shaders.stamp
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/build_shaders.py ${MA_ROOT}/Lib/Fang2/dx ${MA_SHADER_DIR}
    COMMAND ${CMAKE_COMMAND} -E touch ${MA_SHADER_DIR}/shaders.stamp
    DEPENDS ${MA_SHADER_SRCS} ${CMAKE_SOURCE_DIR}/tools/build_shaders.py
    COMMENT "Assembling Fang shaders"
    VERBATIM)
add_custom_target(ma_shaders DEPENDS ${MA_SHADER_DIR}/shaders.stamp)

add_library(fang2 STATIC ${FANG2_SOURCES} port/compat/d3d8_compat.cpp port/gcaudio.cpp port/gcdata.cpp port/gcmesh.cpp port/pc_input.cpp port/discord_rpc.cpp)
target_compile_definitions(fang2 PRIVATE MA_PC_INPUT)
add_dependencies(fang2 ma_shaders)
ma_common_settings(fang2)
target_link_libraries(fang2 PUBLIC smallamx)

# ---------------------------------------------------------------------------
# The game (compile check first; the MFC launcher gets replaced later)
# ---------------------------------------------------------------------------
set(MA_GAME_SOURCES ${MA_SOURCES})
# ma/App/ma/win is the MFC launcher/tools shell; it is replaced rather than ported.
list(FILTER MA_GAME_SOURCES EXCLUDE REGEX "^ma/App/ma/win/")
add_library(ma_game STATIC ${MA_GAME_SOURCES})
target_compile_definitions(ma_game PRIVATE MA_PC_INPUT)
ma_common_settings(ma_game)
target_include_directories(ma_game PRIVATE ${MA_ROOT}/App/ma ${MA_ROOT}/App/ma/win)
target_link_libraries(ma_game PUBLIC fang2)

# ---------------------------------------------------------------------------
# The executable: plain Win32 entry point (replaces the MFC launcher)
# ---------------------------------------------------------------------------
add_executable(ma_port port/main_win.cpp port/screenshot_port.cpp port/res/ma_port.rc)
ma_common_settings(ma_port)
# A windowed app: no console window (main_win.cpp's -console opens one). main() stays the entry point.
target_link_options(ma_port PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup
    $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)
target_include_directories(ma_port PRIVATE ${MA_ROOT}/App/ma ${MA_ROOT}/App/ma/win)
target_link_libraries(ma_port PRIVATE
    ma_game fang2 smallamx
    ${MA_ROOT}/Lib/Bink/binkw32.lib
    d3d9 dinput8 dxguid dsound winmm ws2_32 msacm32
    user32 gdi32 advapi32 ole32 shell32 comdlg32 dbghelp)
add_custom_command(TARGET ma_port POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MA_ROOT}/App/ma/win/binkw32.dll $<TARGET_FILE_DIR:ma_port>
    COMMENT "Copying binkw32.dll")

# Focus/deadzone/button mapping checks; no game data or controller required.
add_executable(ma_input_tests EXCLUDE_FROM_ALL tools/test_pc_input.cpp port/pc_input.cpp)
ma_common_settings(ma_input_tests)
target_link_libraries(ma_input_tests PRIVATE user32)

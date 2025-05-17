cmake_minimum_required(VERSION 3.18)

project(d2helper VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)

add_compile_options(/W3 /WX /Zi /wd4819 /D_CRT_SECURE_NO_WARNINGS)

option(WITH_REGSIM "Enable registry sandbox" ON)

# most MODs do not alter the key table
option(PD2_SUPPORT "Enable PD2 support" OFF)

add_subdirectory(D2Dll)
add_subdirectory(common)
add_subdirectory(modules)
add_subdirectory(detours)

add_library(d2helper SHARED
    DllMain.cpp
    d2helper.def
)

add_library(helper STATIC Install.cpp)

target_link_libraries(helper D2Dll common modules)
target_link_libraries(d2helper helper)

if (WITH_REGSIM)
    add_subdirectory(regsim)
    target_compile_definitions(d2helper PRIVATE REGSIM_ON)
    target_link_libraries(d2helper regsim)
endif()

add_subdirectory(test)

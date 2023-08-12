cmake_minimum_required(VERSION 3.18)

project(d2helper VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)

add_compile_options(/W3 /WX /Zi /wd4819 /D_CRT_SECURE_NO_WARNINGS)

add_library(d2helper SHARED
    DllMain.cpp
    d2helper.def
    )

add_library(helper STATIC
    log.cpp
    d2ptrs_impl.cpp D2CallStub.cpp
    HelpFunc.cpp Install.cpp loader.cpp
    config.cpp D2Utils.cpp d2hmodule.cpp
    MapHelper.cpp FastCast.cpp Event.cpp Timer.cpp WinTM.cpp

    KeyModule.cpp hotkey.cpp
)

target_link_libraries(d2helper helper)

add_subdirectory(test)

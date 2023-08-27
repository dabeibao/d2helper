add_library(modules OBJECT
    d2hmodule.cpp
    Event.cpp Timer.cpp WinTM.cpp
    KeyModule.cpp hotkey.cpp
    FastCast.cpp
)

target_include_directories(modules INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(modules common D2Dll)

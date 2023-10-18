add_library(modules OBJECT
    d2hmodule.cpp
    Event.cpp Timer.cpp WinTM.cpp
    KeyModule.cpp hotkey.cpp
    FastCast.cpp
    #AutoPotion.cpp
    hackscript.cpp hackscriptImpl.cpp
)

target_include_directories(modules INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(modules common D2Dll)

add_library(loadscript SHARED hackscriptImpl.cpp)
target_compile_definitions(loadscript PRIVATE BUILD_HACKSCRIPT)
target_link_libraries(loadscript common D2Dll)
set_target_properties(loadscript PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

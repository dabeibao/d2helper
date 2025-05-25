set(SOURCES
    d2hmodule.cpp
    Event.cpp Timer.cpp WinTM.cpp
    KeyModule.cpp hotkey.cpp
    FastCast.cpp
    #AutoPotion.cpp
    hackscript.cpp hackscriptImpl.cpp
)

add_library(modules OBJECT
    ${SOURCES}
)

target_include_directories(modules INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(modules common D2Dll)

add_library(pd2modules OBJECT
    ${SOURCES}
    PD2Support.cpp
)

target_compile_definitions(pd2modules PRIVATE -DPD2_SUPPORT)
target_include_directories(pd2modules INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(pd2modules common D2Dll)


add_library(loadscript SHARED hackscriptImpl.cpp)
target_compile_definitions(loadscript PRIVATE BUILD_HACKSCRIPT)
target_link_libraries(loadscript common D2Dll)
set_target_properties(loadscript PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

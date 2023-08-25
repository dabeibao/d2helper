add_library(regsim STATIC
    regsim.cpp
)

target_include_directories(regsim INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(regsim common detours)

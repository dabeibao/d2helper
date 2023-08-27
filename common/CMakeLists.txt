add_library(common OBJECT
    log.cpp
    HelpFunc.cpp
    loader.cpp
    config.cpp
)

target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common D2Dll)

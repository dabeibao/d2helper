add_library(D2Dll OBJECT
    d2ptrs_impl.cpp
    D2Utils.cpp
    D2CallStub.cpp
)

target_include_directories(D2Dll PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

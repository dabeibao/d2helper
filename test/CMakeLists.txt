add_executable(test_helper test.cpp)
target_link_libraries(test_helper d2helper)
set_target_properties(test_helper PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

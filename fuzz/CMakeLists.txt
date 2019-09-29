add_custom_target(fuzz)

add_definitions(-g -fsanitize=address,fuzzer -DFUZZING=1)

add_executable(fuzz_packet_parser
    EXCLUDE_FROM_ALL
    "${CMAKE_HOME_DIRECTORY}/fuzz/gdb/packet_parser.c"
    "${CMAKE_HOME_DIRECTORY}/src/gdb/packet.c"
)
set_target_properties(fuzz_packet_parser PROPERTIES RUNTIME_OUTPUT_DIRECTORY   "${CMAKE_BINARY_DIR}/fuzz")
add_dependencies(fuzz fuzz_packet_parser)
target_link_libraries(fuzz_packet_parser -g -fsanitize=address,fuzzer -DFUZZING=1)
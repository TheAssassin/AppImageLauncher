add_subdirectory(commands)

add_executable(ail-cli cli_main.cpp)
target_link_libraries(ail-cli PUBLIC cli_commands)

add_subdirectory(commands)

add_executable(ail-cli cli_main.cpp)
target_link_libraries(ail-cli PUBLIC cli_commands)

set_property(
    TARGET ail-cli
    PROPERTY INSTALL_RPATH ${_rpath}
)

install(
    TARGETS ail-cli
    DESTINATION ${_bindir} COMPONENT APPIMAGELAUNCHER_CLI
)

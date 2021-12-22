add_library(cli_commands STATIC
    Command.h
    CommandFactory.h CommandFactory.cpp
    exceptions.h
    IntegrateCommand.h IntegrateCommand.cpp
    UnintegrateCommand.h UnintegrateCommand.cpp
    WouldIntegrateCommand.h WouldIntegrateCommand.cpp
)
target_link_libraries(cli_commands PUBLIC Qt5::Core shared cli_logging libappimage)
target_include_directories(cli_commands PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

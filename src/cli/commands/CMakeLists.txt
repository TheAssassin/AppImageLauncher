add_library(cli_commands STATIC Command.h CommandFactory.cpp CommandFactory.h IntegrateCommand.cpp IntegrateCommand.h UnintegrateCommand.h UnintegrateCommand.cpp exceptions.h)
target_link_libraries(cli_commands PUBLIC Qt5::Core shared cli_logging)
target_include_directories(cli_commands PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

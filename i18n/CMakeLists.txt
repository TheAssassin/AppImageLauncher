# lupdate must be installed and available
find_program(LUPDATE lupdate)

# create target calling custom commands
add_custom_target(i18n)

# create/update AppImageLauncher strings
function(ail_generate_translations)
    if(ARGC LESS 2)
        message(ERROR "ail_generate_translations called with wrong amount of arguments")
    endif()

    add_custom_command(
        TARGET i18n POST_BUILD
        COMMAND ${LUPDATE} "${ARGV0}" -ts "${ARGV1}"
        MAIN_DEPENDENCY "${ARGV0}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

    )
endfunction()

ail_generate_translations(../src/main.cpp        main.ts)
ail_generate_translations(../src/remove_main.cpp remove.ts)
ail_generate_translations(../src/update_main.cpp update.ts)
ail_generate_translations(../src/shared.cpp      shared.ts)

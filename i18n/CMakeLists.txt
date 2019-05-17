# lupdate must be installed and available
check_program(NAME lupdate)
check_program(NAME lrelease)

# create target calling custom commands
add_custom_target(i18n)

# create/update AppImageLauncher strings
function(ail_generate_translations)
    if(ARGC LESS 2)
        message(ERROR "ail_generate_translations called with wrong amount of arguments")
    endif()

    add_custom_command(
        TARGET i18n POST_BUILD
        COMMAND ${LUPDATE} "${ARGV0}" -ts "${ARGV1}" -locations relative
        MAIN_DEPENDENCY "${ARGV0}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

    )
endfunction()

ail_generate_translations(../src/ ui.en.ts)

file(GLOB TS_FILES *.ts)

foreach(TS_FILE IN LISTS TS_FILES)
    get_filename_component(TS_FILENAME ${TS_FILE} NAME)
    ail_generate_translations(../src/ ${TS_FILENAME})
endforeach()

add_custom_command(
    TARGET i18n POST_BUILD
    COMMAND ./auto-translate.py ui.en.ts
    MAIN_DEPENDENCY "${ARGV0}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


add_custom_target(l10n ALL)

# generate Qt qm files from translation source files
function(ail_generate_qm)
    if(ARGC LESS 2)
        message(ERROR "ail_generate_translations called with wrong amount of arguments")
    endif()

    add_custom_command(
        TARGET l10n POST_BUILD
        COMMAND ${LRELEASE} "${ARGV0}" -qm "${ARGV1}"
        MAIN_DEPENDENCY "${ARGV0}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

    )
endfunction()

file(GLOB TS_FILES *.ts)

set(target_dir ${CMAKE_CURRENT_BINARY_DIR}/generated/l10n)

foreach(TS_FILE IN LISTS TS_FILES)
    get_filename_component(TS_FILENAME ${TS_FILE} NAME)
    string(REPLACE .ts .qm QM_FILENAME ${TS_FILENAME})
    ail_generate_qm(${TS_FILENAME} ${target_dir}/${QM_FILENAME})
endforeach()

# deploy JSON files
file(GLOB JSON_FILES *.json)

foreach(JSON_FILE IN LISTS JSON_FILES)
    get_filename_component(JSON_FILENAME ${JSON_FILE} NAME)
    add_custom_command(
        TARGET l10n POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${JSON_FILENAME} ${target_dir}/${JSON_FILENAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        MAIN_DEPENDENCY ${JSON_FILE}
    )
endforeach()


# empty directories aren't tracked by Git
# therefore the directory needs to be created by CMake
file(MAKE_DIRECTORY ${target_dir})

install(
    DIRECTORY ${target_dir}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/appimagelauncher COMPONENT APPIMAGELAUNCHER
)

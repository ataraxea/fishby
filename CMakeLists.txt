cmake_minimum_required(VERSION 3.20)

project(fishby
    VERSION 0.1.0
    DESCRIPTION "A tiny terminal aquarium"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FISHBY_HARDENING
    "Enable compiler security hardening" ON)

option(FISHBY_SANITIZERS
    "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

# Common application sources
set(FISHBY_SOURCES
    src/main.c
    src/fish.c
    src/bubbles.c
    src/aquarium.c
    src/food.c
    src/pet.c
    src/cli.c
    src/signals.c
    src/platform_time.c
    src/platform_terminal.c
)

# Select platform-specific storage backend
if(WIN32)
    list(APPEND FISHBY_SOURCES
        src/state_windows.c
    )
else()
    list(APPEND FISHBY_SOURCES
        src/state.c
    )
endif()

# Compiler configuration
function(fishby_configure_target target)
    target_include_directories(${target}
        PRIVATE include
    )

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wformat=2
            -Werror=format-security
        )
    endif()

    # Linux ELF hardening
    if(FISHBY_HARDENING
       AND CMAKE_SYSTEM_NAME STREQUAL "Linux"
       AND NOT FISHBY_SANITIZERS)

        target_compile_options(${target} PRIVATE
            -fstack-protector-strong
            -fstack-clash-protection
            -fPIE
        )

        target_compile_definitions(${target} PRIVATE
            $<$<CONFIG:Release>:_FORTIFY_SOURCE=3>
            $<$<CONFIG:RelWithDebInfo>:_FORTIFY_SOURCE=3>
        )

        target_link_options(${target} PRIVATE
            -pie
            -Wl,-z,relro
            -Wl,-z,now
            -Wl,-z,noexecstack
        )

    elseif(FISHBY_HARDENING
           AND WIN32
           AND MINGW
           AND NOT FISHBY_SANITIZERS)

        target_compile_options(${target} PRIVATE
            -fstack-protector-strong
        )

        target_link_options(${target} PRIVATE
            -Wl,--dynamicbase
            -Wl,--nxcompat
        )

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_link_options(${target} PRIVATE
                -Wl,--high-entropy-va
            )
        endif()

    elseif(FISHBY_HARDENING
           AND APPLE
           AND NOT FISHBY_SANITIZERS)

        target_compile_options(${target} PRIVATE
            -fstack-protector-strong
        )
    endif()

    # Debug instrumentation
    if(FISHBY_SANITIZERS)
        if(WIN32)
            message(FATAL_ERROR
                "Sanitizer builds are not configured for Windows")
        endif()

        target_compile_options(${target} PRIVATE
            -O1
            -g3
            -fno-omit-frame-pointer
            -fno-optimize-sibling-calls
            -fsanitize=address,undefined
            -fno-sanitize-recover=all
        )

        target_link_options(${target} PRIVATE
            -fsanitize=address,undefined
        )
    endif()
endfunction()

# Main executable
add_executable(fishby ${FISHBY_SOURCES})

fishby_configure_target(fishby)

# Terminal dependencies
if(WIN32)
    set(PDCURSES_ROOT "" CACHE PATH
        "Path to PDCurses installation or source tree")

    find_path(PDCURSES_INCLUDE_DIR
        NAMES curses.h
        HINTS "${PDCURSES_ROOT}"
        PATH_SUFFIXES include
        REQUIRED
    )

    find_library(PDCURSES_LIBRARY
        NAMES pdcurses_wincon pdcurses pdcurses.a
        HINTS "${PDCURSES_ROOT}"
        PATH_SUFFIXES lib wincon
        REQUIRED
    )

    target_include_directories(fishby PRIVATE
        "${PDCURSES_INCLUDE_DIR}"
    )

    target_link_libraries(fishby PRIVATE
        "${PDCURSES_LIBRARY}"
    )

else()
    set(CURSES_NEED_NCURSES TRUE)

    find_package(Curses REQUIRED)

    target_include_directories(fishby PRIVATE
        ${CURSES_INCLUDE_DIRS}
    )

    target_link_libraries(fishby PRIVATE
        ${CURSES_LIBRARIES}
        m
    )
endif()

# Automated tests
include(CTest)

if(BUILD_TESTING)

    add_executable(test_pet
        tests/test_pet.c
        src/pet.c
    )

    fishby_configure_target(test_pet)

    if(NOT WIN32)
        target_link_libraries(test_pet PRIVATE m)
    endif()

    target_compile_options(test_pet PRIVATE
        -UNDEBUG
    )

    add_test(
        NAME pet_unit_tests
        COMMAND test_pet
    )

    add_test(
        NAME cli_help
        COMMAND fishby --help
    )

    add_test(
        NAME cli_version
        COMMAND fishby --version
    )

    if(UNIX)

        add_executable(test_state
            tests/test_state.c
            src/state.c
            src/pet.c
        )

        fishby_configure_target(test_state)

        target_compile_options(test_state PRIVATE
            -UNDEBUG
        )

        target_link_libraries(test_state PRIVATE m)

        add_test(
            NAME state_security_tests
            COMMAND test_state
        )

        find_program(BASH_EXECUTABLE bash REQUIRED)

        add_test(
            NAME cli_regression_tests
            COMMAND "${BASH_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cli.sh"
                $<TARGET_FILE:fishby>
        )

    endif()
endif()

# Installation
install(TARGETS fishby
    RUNTIME DESTINATION bin
)

# Build summary
message(STATUS "fishby configuration")
message(STATUS "  Platform:    ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler:    ${CMAKE_C_COMPILER_ID}")
message(STATUS "  Build type:  ${CMAKE_BUILD_TYPE}")
message(STATUS "  Hardening:   ${FISHBY_HARDENING}")
message(STATUS "  Sanitizers:  ${FISHBY_SANITIZERS}")
message(STATUS "  Tests:       ${BUILD_TESTING}")

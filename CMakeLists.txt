cmake_minimum_required(VERSION 3.16)
project(SpaceInvadersEmulator C)

# ── Language standard ────────────────────────────────────────────────────────
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Default build type ────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ── Compiler warnings ─────────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -fsanitize=address,undefined)
        add_link_options(-fsanitize=address,undefined)
    else()
        add_compile_options(-O2)
    endif()
endif()

# ── SDL2 detection ────────────────────────────────────────────────────────────
# Primary: cmake's own FindSDL2 (ships with CMake >= 3.25) or vcpkg/conan toolchain
find_package(SDL2 QUIET)

# Fallback: pkg-config (common on Linux)
if(NOT SDL2_FOUND)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(SDL2 REQUIRED sdl2)
        set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
        set(SDL2_LIBRARIES    ${SDL2_LIBRARIES})
    else()
        message(FATAL_ERROR
            "SDL2 not found. Install libsdl2-dev (Linux) or SDL2 via MSYS2:\n"
            "  Linux:   sudo apt install libsdl2-dev\n"
            "  MSYS2:   pacman -S mingw-w64-ucrt-x86_64-SDL2\n"
            "  vcpkg:   vcpkg install sdl2")
    endif()
endif()

# ── Sources ───────────────────────────────────────────────────────────────────
file(GLOB SOURCES "src/*.c")
file(GLOB HEADERS "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})

# On MinGW, SDL2main must be linked before SDL2
if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE mingw32 SDL2main)
endif()

# ── Post-build: copy assets next to the binary ────────────────────────────────
# rom/ directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/rom")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/rom"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/rom"
        COMMENT "Copying rom/ assets"
    )
endif()

# sounds/ directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sounds")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/sounds"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/sounds"
        COMMENT "Copying sounds/ assets"
    )
endif()

# SDL2.dll (Windows only — copy from source root if present)
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll"
        COMMENT "Copying SDL2.dll"
    )
endif()

# ── Install rules (optional) ──────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY rom    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
install(DIRECTORY sounds DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

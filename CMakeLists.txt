cmake_minimum_required(VERSION 3.14...3.21)
project(CHIP-8 LANGUAGES CXX)

option(USE_FALLBACK "Download and build dependencies from source if not found." OFF)

add_executable(chip-8 "src/main.cpp" "src/CHIP8.cpp")

target_compile_features(chip-8 PRIVATE cxx_std_20)
target_compile_definitions(chip-8 PRIVATE "$<$<CONFIG:Release>:DISABLE_RAYLIB_LOGS>")

find_package(argparse QUIET)
find_package(fmt QUIET)
find_package(raylib QUIET)

if (USE_FALLBACK)
	if (NOT argparse_FOUND)
		include(FetchContent)
		FetchContent_Declare(
			argparse
			GIT_REPOSITORY https://github.com/p-ranav/argparse
			GIT_TAG v2.2
		)
		FetchContent_MakeAvailable(argparse)
	endif()
	if (NOT fmt_FOUND)
		include(FetchContent)
		FetchContent_Declare(
			fmt
			GIT_REPOSITORY https://github.com/fmtlib/fmt
			GIT_TAG 8.0.1
		)
		FetchContent_MakeAvailable(fmt)
	endif()
	if (NOT raylib_FOUND)
		include(FetchContent)
		FetchContent_Declare(
			raylib
			GIT_REPOSITORY https://github.com/raysan5/raylib
			GIT_TAG 3.7.0
		)
		FetchContent_MakeAvailable(raylib)
		target_link_libraries(chip-8 PRIVATE raylib)
	endif()
endif()

target_link_libraries(chip-8 PRIVATE argparse::argparse fmt::fmt)
target_include_directories(chip-8 PRIVATE ${raylib_INCLUDE_DIRS})
target_link_libraries(chip-8 PRIVATE ${raylib_LIBRARIES})

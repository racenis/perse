cmake_minimum_required(VERSION 3.10)
project(perse_backend C)

set(CMAKE_C_STANDARD 99)

add_library(perse_backend_shared SHARED win32.c)
target_include_directories(perse_backend_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(perse_backend_static STATIC win32.c)
target_include_directories(perse_backend_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Proper output names for cross-platform
set_target_properties(perse_backend_shared PROPERTIES
    OUTPUT_NAME "backend"
    PREFIX ""  # avoid libbackend.dll on Windows
	ARCHIVE_OUTPUT_NAME ""
)

set_target_properties(perse_backend_static PROPERTIES
    OUTPUT_NAME "backend"
    PREFIX ""  # avoid libbackend.a in Emscripten
)

# Special case for Emscripten: don’t build shared lib
if (EMSCRIPTEN)
    set_target_properties(perse_backend_shared PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

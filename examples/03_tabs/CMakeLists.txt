cmake_minimum_required(VERSION 3.10)
project(example CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# TODO: add EMSCRIPTEN backend
if(WIN32)
    set(BACKEND_DIR "win32")
    set(BACKEND_LIB "backend.dll")
elseif(UNIX AND NOT APPLE)
    set(BACKEND_DIR "motif")
    set(BACKEND_LIB "backend.so")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

add_executable(example example.cpp)

target_link_libraries(example 
    PRIVATE
    persefrontend
    perse
)

target_link_directories(example PRIVATE
    ../../src/library/build
    ../../src/frontend/cpp/build
)

add_custom_command(TARGET example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/../../src/backend/${BACKEND_DIR}/build/${BACKEND_LIB}"
        "$<TARGET_FILE_DIR:example>/${BACKEND_LIB}"
    COMMENT "Copying ${BACKEND_LIB} to output directory"
)
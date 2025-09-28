cmake_minimum_required(VERSION 3.10)
project(PerseFrontend CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Build a static library from source files
add_library(persefrontend STATIC
    perse.h
    perse.cpp
	
    property.h

    widget.h
    widget.cpp
	
    hooks.h
    hooks.cpp
)
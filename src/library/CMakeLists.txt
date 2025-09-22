cmake_minimum_required(VERSION 3.10)
project(PerseLibrary C)

# Build a static library from source files
add_library(perse STATIC
    perse.c
    property.h
    property.c
    widget.h
    widget.c
	layout.h
	layout.c
)
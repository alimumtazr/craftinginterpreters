cmake_minimum_required(VERSION 3.15)

project(clox
  VERSION 1.0.0
  DESCRIPTION "A bytecode virtual machine for the Lox language (Crafting Interpreters, Part III)"
  LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

add_executable(clox
  main.c
  chunk.c
  memory.c
  value.c
  object.c
  table.c
  debug.c
  vm.c
  compiler.c
  scanner.c)

target_sources(clox PRIVATE
  common.h
  chunk.h
  memory.h
  value.h
  object.h
  table.h
  debug.h
  vm.h
  compiler.h
  scanner.h)

if(MSVC)
  target_compile_options(clox PRIVATE /W4)
else()
  target_compile_options(clox PRIVATE -Wall -Wextra)
endif()

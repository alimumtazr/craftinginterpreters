cmake_minimum_required(VERSION 3.15)

project(lox
  VERSION 1.0.0
  DESCRIPTION "A tree-walking interpreter for the Lox language (Crafting Interpreters, Part II)"
  LANGUAGES CXX)

# --- Language standard ---------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a Release build when the user hasn't picked one (e.g. plain
# single-config generators like Makefiles / Ninja).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# --- Interpreter executable 
# These are exactly the translation units that make up the interpreter.
# (AstPrinter.cpp is an optional debug utility and is intentionally not
# linked into the interpreter, matching how we build it by hand.)
add_executable(lox
  lox.cpp
  scanner.cpp
  Parser.cpp
  Interpreter.cpp
  LoxFunction.cpp
  Resolver.cpp
  LoxClass.cpp
  LoxInstance.cpp)

# Headers are listed only so they show up in IDEs; they aren't compiled
# directly.
target_sources(lox PRIVATE
  Lox.h
  TokenType.h
  Token.h
  Scanner.h
  Expr.h
  Stmt.h
  Parser.h
  Environment.h
  RuntimeError.h
  ReturnException.h
  Interpreter.h
  Resolver.h
  LoxCallable.h
  LoxFunction.h
  LoxClass.h
  LoxInstance.h)

# --- Warnings ------------------------------------------------------------
if(MSVC)
  target_compile_options(lox PRIVATE /W4)
else()
  target_compile_options(lox PRIVATE -Wall -Wextra)
endif()

# --- Tests ---------------------------------------------------------------
# Convenience target that runs the chapterwise PowerShell test suite against
# the freshly built interpreter:  cmake --build . --target run_tests
find_program(POWERSHELL_EXE
  NAMES pwsh powershell
  DOC "PowerShell executable used to run the test suite")

if(POWERSHELL_EXE)
  add_custom_target(run_tests
    COMMAND "${POWERSHELL_EXE}" -ExecutionPolicy Bypass
            -File "${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests.ps1"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    USES_TERMINAL
    COMMENT "Running chapterwise Lox test suite")
endif()

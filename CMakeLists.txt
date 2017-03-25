cmake_minimum_required(VERSION 3.6)
project(FastRegExp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

include(ExternalProject)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckIncludeFileCXX)

add_subdirectory(thirdparty/asmjit)

ExternalProject_Add(libasmjit
        URL thirdparty/asmjit
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/asmjit
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DASMJIT_STATIC=TRUE -DASMJIT_DISABLE_TEXT=TRUE -DASMJIT_DISABLE_LOGGING=TRUE)

ExternalProject_Get_Property(libasmjit INSTALL_DIR)
link_directories(${INSTALL_DIR}/src/libasmjit-build)
include_directories(${INSTALL_DIR}/include)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(FAST_REG_EXP
        src/Parser.cpp
        include/AST.h
        include/Parser.h)

add_executable(FastRegExp ${FAST_REG_EXP} src/main.cpp)
add_dependencies(FastRegExp libasmjit)
target_link_libraries(FastRegExp libasmjit.a)
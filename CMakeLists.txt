# ########## Project setup ##########
PROJECT(sdatxtract)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

# SDATxtract Version
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 0)
SET(VERSION_MICRO 1)
SET(SDATXTRACT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}")

# Find Macros
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

INCLUDE(CMakeDependentOption)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckIncludeFile)
INCLUDE(TestBigEndian)

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_BINARY_DIR}/include")
CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/src/config.h.cmake" "${CMAKE_BINARY_DIR}/include/config.h")


FILE(GLOB SOURCES "src/*.c" "src/decoder/*.c")

ADD_EXECUTABLE(sdatxtract ${SOURCES})

if(MSVC)
  TARGET_COMPILE_OPTIONS(sdatxtract PRIVATE /W4 /WX- -D_CRT_SECURE_NO_WARNINGS)
else(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  TARGET_COMPILE_OPTIONS(sdatxtract PRIVATE -Wall -Wconversion -Wsign-conversion -Wno-long-long -pedantic)
endif()

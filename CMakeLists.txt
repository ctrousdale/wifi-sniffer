# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.28)

# Set C++ standard before including ESP-IDF
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Instead of setting global flags, we'll set them per component

# ESP-IDF include
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(wifi-sniffer)

# DO NOT set global options that affect C files:
# idf_build_set_property(COMPILE_OPTIONS "-fmodules-ts" APPEND LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)
project(grdcl_core LANGUAGES CXX)

if(POLICY CMP0148)
    cmake_policy(SET CMP0148 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)

pybind11_add_module(grdecl_core 
    src/bindings.cpp 
    src/parser.cpp
    src/grid_builder.cpp
    src/obj_exporter.cpp
)
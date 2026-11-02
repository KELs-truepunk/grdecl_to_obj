cmake_minimum_required(VERSION 3.15)
project(grdecl_to_obj)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)

pybind11_add_module(grdecl_core
    src/bindings.cpp
    src/parser.cpp
    )

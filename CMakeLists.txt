cmake_minimum_required(VERSION 3.15)
project(grdecl_to_obj LANGUAGES CXX)

if(POLICY CMP0148)
    cmake_policy(SET CMP0148 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)

pybind11_add_module(grdecl_core 
    src/bindings.cpp 
    src/parser/parser.cpp
    src/grid_builder/grid_builder.cpp
    src/obj_exporter/obj_exporter.cpp
    src/stl_exporter/stl_exporter.cpp
    src/ply_exporter/ply_exporter.cpp
    src/vtu_exporter/vtu_exporter.cpp
    src/vtk_exporter/vtk_exporter.cpp
)
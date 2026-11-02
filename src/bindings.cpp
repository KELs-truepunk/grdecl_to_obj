#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "parser.hpp"
#include "grid_builder.hpp"
namespace py = pybind11;

PYBIND11_MODULE(grdecl_core, m) {
    py::class_<Parser>(m, "Parser")
        .def(py::init<const std::string&>())           // конструктор с указанием пути к файлу
        .def("load", &Parser::load)                    // метод загрузки файла
        .def("get_specgrid", &Parser::get_specgrid)    // метод получения размеров сетки
        .def("parse_keyword", &Parser::parse_keyword); // метод парсинга ключевого слова
        
    py::class_<GridBuilder>(m, "GridBuilder")
        .def_static("build_mesh_grid", &GridBuilder::build_mesh_grid); // статический метод построения сетки
}
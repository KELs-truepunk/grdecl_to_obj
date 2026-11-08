#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
// подключаем заголовочные файлы для классов Parser, GridBuilder и ObjExporter
#include "parser/parser.hpp"
#include "grid_builder/grid_builder.hpp"
#include "obj_exporter/obj_exporter.hpp"
#include "stl_exporter/stl_exporter.hpp"
#include "ply_exporter/ply_exporter.hpp"
#include "vtu_exporter/vtu_exporter.hpp"
#include "vtk_exporter/vtk_exporter.hpp"

namespace py = pybind11;
//пайбинд11 позволяет сделать ручки моих си++ классов в питоне
PYBIND11_MODULE(grdecl_core, m) {
    py::class_<Parser>(m, "Parser")
        .def(py::init<const std::string&>())           // конструктор с указанием пути к файлу
        .def("load", &Parser::load)                    // метод загрузки файла
        .def("get_specgrid", &Parser::get_specgrid)    // метод получения размеров сетки
        .def("parse_keyword", &Parser::parse_keyword); // метод парсинга ключевого слова
        
    py::class_<GridBuilder>(m, "GridBuilder")
        .def_static("build_mesh_grid", &GridBuilder::build_mesh_grid,
                    py::arg("dims"), 
                    py::arg("coords"), 
                    py::arg("zcorn"), 
                    py::arg("actnum") = std::vector<float>{}, 
                    py::arg("z_scale") = 1.0f);                // статический метод построения сетки
    py::class_<ObjExporter>(m, "ObjExporter")
        .def_static("export_to_obj", &ObjExporter::export_to_obj); // статический метод экспорта в формат OBJ
    py::class_<StlExporter>(m, "StlExporter")
        .def_static("export_to_stl", &StlExporter::export_to_stl); // статический метод экспорта в формат STL
    py::class_<PlyExporter>(m, "PlyExporter")
        .def_static("export_to_ply", &PlyExporter::export_to_ply); // статический метод экспорта в формат PLY
    py::class_<VtuExporter>(m, "VtuExporter")
        .def_static("export_to_vtu", &VtuExporter::export_to_vtu); // статический метод экспорта в формат VTU
    py::class_<VtkExporter>(m, "VtkExporter")
        .def_static("export_to_vtk", &VtkExporter::export_to_vtk); // статический метод экспорта в формат VTK
}
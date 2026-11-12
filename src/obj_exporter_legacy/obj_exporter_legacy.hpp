#pragma once
#include <vector>
#include <string>

// Простой экспортёр, который записывает вершины сетки в файл Wavefront OBJ.
class ObjExporterLegacy {
    public:
    // Вершины передаются подряд: по восемь точек с координатами X, Y, Z на ячейку.
    static bool export_to_obj(const std::string& filepath, const std::vector<float>& vertices);

};
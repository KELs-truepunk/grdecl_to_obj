#pragma once
#include <vector>
#include <string>

// Экспортирует геометрию сетки в треугольную поверхностную модель STL.
class StlExporter{
    public:
    // Записывает сетку по указанному пути; при успехе возвращает true.
    static bool export_to_stl(const std::string& filepath,
    std::vector<float>& vertices);
     
};
#pragma once
#include <string>
#include <vector>

// Сохраняет сетку из гексаэдрических ячеек в текстовом формате VTK.
class VtkExporter {
public:
    // Возвращает true, если файл удалось создать и записать.
    static bool export_to_vtk(const std::string& filepath,
                              const std::vector<float>& vertices);
};
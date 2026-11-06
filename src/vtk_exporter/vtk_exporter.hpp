#pragma once
#include <string>
#include <vector>

class VtkExporter {
public:
    static bool export_to_vtk(const std::string& filepath,
                              const std::vector<float>& vertices);
};
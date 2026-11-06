#pragma once
#include <string>
#include <vector>

class VtuExporter {
public:
    static bool export_to_vtu(const std::string& filepath,
                              const std::vector<float>& vertices);
};
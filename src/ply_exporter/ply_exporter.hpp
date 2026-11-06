#pragma once 
#include <string>
#include <vector>
class PlyExporter {
public:
    // бинарный режим по умолчанию выключен
    //бинраный режим в литл-эндиан формате(меньше места), иначе в АSCII формате
    static bool export_to_ply(const std::string& filepath, const std::vector<float>& vertices, 
        bool binary = false );
    }
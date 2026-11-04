#pragma once
#include <vector>
#include <string>

class ObjExporter {
    public:
    static bool export_to_obj(const std::string& filepath, const std::vector<float>& vertices);

};
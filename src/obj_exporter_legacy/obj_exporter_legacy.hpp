#pragma once
#include <vector>
#include <string>

class ObjExporterLegacy {
    public:
    static bool export_to_obj(const std::string& filepath, const std::vector<float>& vertices);

};
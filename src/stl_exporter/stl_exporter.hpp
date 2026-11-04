#pragma once
#include <vector>
#include <string>

class StlExporter{
    public:
    static bool export_to_stl(const std::string& filepath,
    std::vector<float>& vertices);
     
};
#pragma once
#include <vector>
#include <string>
#include <map>
#include <array>

struct ObjFace { int v0, v1, v2; };
struct ObjMesh { std::vector<float> vertices; std::vector<ObjFace> faces; };

class ObjExporter {
public:
    static bool export_optimized_obj(
        const std::string& filepath,
        const std::vector<float>& raw_vertices,
        const std::vector<int>& dims,
        const std::vector<float>& actnum, // <--- ДОБАВЛЕНО
        bool add_base_plate = false,
        float base_thickness = 5.0f
    );
};
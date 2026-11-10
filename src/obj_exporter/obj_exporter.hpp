#pragma once
#include <vector>
#include <string>
#include <map>
#include <array>

// структура треугольной грани
struct ObjFace {
    int v0, v1, v2;
};

// структура индексного меша
struct ObjMesh {
    std::vector<float> vertices; // Плоский массив x,y,z
    std::vector<ObjFace> faces;  // Треугольники
};

class ObjExporter {
public:
    //принимает сырые данные, возвращает оптимизированный меш
    static bool export_optimized_obj(
        const std::string& filepath,
        const std::vector<float>& raw_vertices,
        const std::vector<int>& dims,
        bool add_base_plate = false,
        float base_thickness = 5.0f
    );
};
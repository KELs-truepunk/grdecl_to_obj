#pragma once
#include <vector>

// Структура для возврата данных (вершины + параметры для раскраски)
struct MeshData {
    std::vector<float> vertices; // x,y,z подряд
    std::vector<float> scalars;  // Значения (например, FLUIDS) для каждой ячейки
};

class GridBuilder {
public:
    /* dims - размеры {NX, NY, NZ}
       coords - массив COORD
       zcorn - массив ZCORN
       actnum - массив активных ячеек (опционально)
       fluids - массив параметров (опционально)
       z_scale - коэффициент вытягивания по Z (опционально) */
    static MeshData build_mesh_grid(
        const std::vector<int>& dims,
        const std::vector<float>& coords,
        const std::vector<float>& zcorn,
        const std::vector<float>& actnum = {}, 
        const std::vector<float>& fluids = {}, 
        float z_scale = 1.0f                   
    );
};               
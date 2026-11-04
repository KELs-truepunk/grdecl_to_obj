#pragma once

#include <vector>

class GridBuilder {
    public:
    /*принимаем размеры (NX, NY, NZ), линии COORD и глубины ZCOORN
    а возвращаем плоский массив вершин (x, y, z) и структуру для PyVista*/
    static std::vector<float> build_mesh_grid(
        const std::vector<int>& dims,
        const std::vector<float>& coords,
        const std::vector<float>& zcoorn
    );
};
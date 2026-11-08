#pragma once
#include <vector>

class GridBuilder {
public:
    /* dims - размеры {NX, NY, NZ}
       coords - массив COORD
       zcorn - массив ZCORN
       actnum - массив активных ячеек (опционально)
       z_scale - коэффициент вытягивания по Z (опционально) */
    static std::vector<float> build_mesh_grid(
        const std::vector<int>& dims,
        const std::vector<float>& coords,
        const std::vector<float>& zcorn,
        const std::vector<float>& actnum = {}, 
        float z_scale = 1.0f                   
    );
};
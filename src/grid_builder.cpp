#include "grid_builder.hpp"
#include <iostream>

std::vector<float> GridBuilder::build_mesh_grid(
    const std::vector<int>& dims,
    const std::vector<float>& coords,
    const std::vector<float>& zcoorn
){
    if(dims.size() < 3) return {};
    int nx = dims[0];
    int ny = dims[1];
    int nz = dims[2];
    std::vector<float> vertices;
    //TODO: написать Corner-Point интерполяции по COORD и ZCORN
    
    std::cout << "[GridBuilder] Building mesh for grid: " << nx << " x " << ny << " x " << nz << std::endl;
    
    return vertices;
}
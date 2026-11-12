#include <cmath>
#include <algorithm>
#include <iostream>
#include "grid_builder.hpp"

// Точка сетки и вертикальный пиллар, соединяющий верхнюю и нижнюю границы.
struct Point3D {float x, y, z;};
struct Pillar {Point3D top; Point3D bot;};

// Для построения реальной сетки нужны оба исходных массива: COORD и ZCORN.
static bool is_data_empty(const std::vector<float>& coords, const std::vector<float>& zcorn) {
    return coords.empty() || zcorn.empty();
}

// В COORD для каждого пиллара подряд хранятся координаты его верхнего и нижнего концов.
static Pillar get_pillar(int pi, int pj, int nx_plus_1, const std::vector<float>& coords) {
    int index = (pj * nx_plus_1 + pi) * 6;
    return {
        {coords[index + 0], coords[index + 1], coords[index + 2]},
        {coords[index + 3], coords[index + 4], coords[index + 5]}
    };
}

// Находим точку на пилларе с заданной глубиной, линейно интерполируя X и Y.
static Point3D interpolate_pillar_at_z(const Pillar& pillar, float target_z) {
    float dz = pillar.bot.z - pillar.top.z;
    // У почти горизонтального пиллара X и Y не меняются вместе с глубиной.
    if (std::abs(dz) < 1e-5f) return {pillar.top.x, pillar.top.y, target_z};
    
    float t = (target_z - pillar.top.z) / dz;
    // Если глубина выходит за пределы пиллара, используем ближайший его конец.
    t = std::max(0.0f, std::min(1.0f, t));
    
    float x = pillar.top.x + t * (pillar.bot.x - pillar.top.x);
    float y = pillar.top.y + t * (pillar.bot.y - pillar.top.y);
    return {x, y, target_z};
}

// Создаём простую регулярную сетку, когда во входных данных нет COORD или ZCORN.
static void generate_dummy_grid(int nx, int ny, int nz, std::vector<float>& vertices, std::vector<float>& scalars) {
    int total_cells = nx * ny * nz;
    vertices.reserve(total_cells * 24);
    scalars.reserve(total_cells);
    // Каждую ячейку записываем восемью углами; координаты заданы в фиксированном масштабе.
    for (int k = 0; k < nz; ++k) {
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                float x0 = i * 20.0f, x1 = (i + 1) * 20.0f;
                float y0 = j * 20.0f, y1 = (j + 1) * 20.0f;
                float z0 = -k * 10.0f, z1 = -(k + 1) * 10.0f;
                float cell_pts[8][3] = {
                    {x0, y0, z0}, {x1, y0, z0}, {x1, y1, z0}, {x0, y1, z0},
                    {x0, y0, z1}, {x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1}
                };
                for (int v = 0; v < 8; ++v) {
                    vertices.push_back(cell_pts[v][0]);
                    vertices.push_back(cell_pts[v][1]);
                    vertices.push_back(cell_pts[v][2]);
                }
                scalars.push_back(0.0f); // Для заглушки нет исходного значения скаляра.
            }
        }
    }
}

// Собираем ячейки по COORD и ZCORN, а для активных ячеек сохраняем значение FLUIDS.
static void generate_grid(int nx, int ny, int nz, 
                        std::vector<float>& vertices,    
                        std::vector<float>& scalars,
                        const std::vector<float>& coords,
                        const std::vector<float>& zcorn,
                        const std::vector<float>& actnum,
                        const std::vector<float>& fluids) 
{    
    int nx_plus_1 = nx + 1;
    int total_cells = nx * ny * nz;
    vertices.reserve(total_cells * 24);
    scalars.reserve(total_cells); // По одному скаляру на каждую добавленную ячейку.
    
    for (int k = 0; k < nz; ++k) {
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                
                // Индекс ячейки одинаково используется для ACTNUM, ZCORN и FLUIDS.
                int cell_idx = (k * ny + j) * nx + i;
                
                // Неактивные ячейки не попадают в результирующую сетку.
                if (!actnum.empty() && cell_idx < actnum.size()) {
                    if (actnum[cell_idx] < 0.5f) continue; 
                }

                int z_idx = cell_idx * 8;
                // Для ячейки нужны восемь значений глубины — по одному на каждый угол.
                if (z_idx + 7 >= static_cast<int>(zcorn.size())) continue;

                // Если FLUIDS отсутствует или короче сетки, используем нулевое значение.
                if (!fluids.empty() && cell_idx < fluids.size()) {
                    scalars.push_back(fluids[cell_idx]);
                } else {
                    scalars.push_back(0.0f);
                }

                // Углы ячейки обходятся по периметру: четыре сверху, затем четыре снизу.
                Pillar p00 = get_pillar(i,     j,     nx_plus_1, coords);
                Pillar p10 = get_pillar(i + 1, j,     nx_plus_1, coords);
                Pillar p11 = get_pillar(i + 1, j + 1, nx_plus_1, coords);
                Pillar p01 = get_pillar(i,     j + 1, nx_plus_1, coords);

                Point3D v[8];
                // Первые четыре глубины ZCORN задают верхние углы ячейки.
                v[0] = interpolate_pillar_at_z(p00, zcorn[z_idx + 0]);
                v[1] = interpolate_pillar_at_z(p10, zcorn[z_idx + 1]);
                v[2] = interpolate_pillar_at_z(p11, zcorn[z_idx + 2]);
                v[3] = interpolate_pillar_at_z(p01, zcorn[z_idx + 3]);

                // Следующие четыре значения задают соответствующие нижние углы.
                v[4] = interpolate_pillar_at_z(p00, zcorn[z_idx + 4]);
                v[5] = interpolate_pillar_at_z(p10, zcorn[z_idx + 5]);
                v[6] = interpolate_pillar_at_z(p11, zcorn[z_idx + 6]);
                v[7] = interpolate_pillar_at_z(p01, zcorn[z_idx + 7]);

                // Координаты хранятся подряд в формате x, y, z для каждой вершины.
                for (int idx = 0; idx < 8; ++idx) {
                    vertices.push_back(v[idx].x);
                    vertices.push_back(v[idx].y);
                    vertices.push_back(v[idx].z);
                }
            }
        }
    }
}

// Проверяем входные размеры, выбираем способ построения и при необходимости масштабируем глубину.
MeshData GridBuilder::build_mesh_grid(
    const std::vector<int>& dims,
    const std::vector<float>& coords,
    const std::vector<float>& zcorn,
    const std::vector<float>& actnum, 
    const std::vector<float>& fluids, 
    float z_scale                     
){
    MeshData result;
    // Без трёх размеров сетки построить нечего.
    if (dims.size() < 3) return result;
    int nx = dims[0], ny = dims[1], nz = dims[2];
    
    if (is_data_empty(coords, zcorn)) {
        std::cout << "[GridBuilder] COORD or ZCORN is empty. Using dummy grid." << std::endl;
        generate_dummy_grid(nx, ny, nz, result.vertices, result.scalars);
    } else {
        generate_grid(nx, ny, nz, result.vertices, result.scalars, coords, zcorn, actnum, fluids);
    }

    // Масштаб применяется только к координате Z каждой вершины.
    if (z_scale != 1.0f) {
        for (size_t i = 2; i < result.vertices.size(); i += 3) {
            result.vertices[i] *= z_scale;
        }
    }
    
    std::cout << "[GridBuilder] Built mesh: " << nx << "x" << ny << "x" << nz 
              << " (Vertices: " << result.vertices.size() / 3 << ")" << std::endl;
    return result;
}
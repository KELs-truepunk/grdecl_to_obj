#include <cmath>
#include <iostream>
#include "grid_builder.hpp"

// Структура для удобной работы с точками в пространстве
struct Point3D {
    float x, y, z;
};

// Структура вертикальной колонки (pillar) в сетке CPG
struct Pillar {
    Point3D top;
    Point3D bot;
};

// Проверка на наличие сырых данных
static bool is_data_empty(const std::vector<float>& coords, const std::vector<float>& zcorn) {
    return coords.empty() || zcorn.empty();
}

// Достаем конкретный пиллар (колонку) по его индексам (pi, pj) в сетке (nx+1) x (ny+1)
static Pillar get_pillar(int pi, int pj, int nx_plus_1, const std::vector<float>& coords) {
    // Индекс в массиве COORD для верхней и нижней точек пиллара
    int index = (pj * nx_plus_1 + pi) * 6;
    // Возвращаем структуру Pillar с верхней и нижней точкой
    return {
        {coords[index + 0], coords[index + 1], coords[index + 2]}, // Top
        {coords[index + 3], coords[index + 4], coords[index + 5]}  // Bot
    };
}

// Линейная интерполяция X и Y на пилларе по известной глубине Z (из ZCORN)
static Point3D interpolate_pillar_at_z(const Pillar& pillar, float target_z) {
    float dz = pillar.bot.z - pillar.top.z;
    if (std::abs(dz) < 1e-5f) {
        return {pillar.top.x, pillar.top.y, target_z};
    }
    
    // Параметр t от 0 (верх) до 1 (низ)
    float t = (target_z - pillar.top.z) / dz;
    
    // Ограничиваем t в пределах отрезка колонки
    t = std::max(0.0f, std::min(1.0f, t));
    // Интерполируем X и Y
    float x = pillar.top.x + t * (pillar.bot.x - pillar.top.x);
    float y = pillar.top.y + t * (pillar.bot.y - pillar.top.y);
    // возвращаем точку на пилларе с заданной глубиной
    return {x, y, target_z};
}

// генерация параметрического бокса (заглушка для тестов, если файлы пустые)
static void generate_dummy_grid(int nx, int ny, int nz, std::vector<float>& vertices) {
    int total_cells = nx * ny * nz; // количество ячеек в сетке
    vertices.reserve(total_cells * 24); // 8 вершин * 3 координаты = 24 значения на ячейку
    for (int k = 0; k < nz; ++k) {
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                // создаем кубическую ячейку с размерами 20x20x10
                float x0 = i * 20.0f, x1 = (i + 1) * 20.0f;
                float y0 = j * 20.0f, y1 = (j + 1) * 20.0f;
                float z0 = -k * 10.0f, z1 = -(k + 1) * 10.0f;
                // создаем массив вершин для кубической ячейки
                float cell_pts[8][3] = {
                    {x0, y0, z0}, {x1, y0, z0}, {x1, y1, z0}, {x0, y1, z0},
                    {x0, y0, z1}, {x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1}
                };
                // добавляем вершины в общий массив
                for (int v = 0; v < 8; ++v) {
                    vertices.push_back(cell_pts[v][0]);
                    vertices.push_back(cell_pts[v][1]);
                    vertices.push_back(cell_pts[v][2]);
                }
            }
        }
    }
}

// основной метод построения сетки
std::vector<float> GridBuilder::build_mesh_grid(
    const std::vector<int>& dims,
    const std::vector<float>& coords,
    const std::vector<float>& zcorn
){
    if (dims.size() < 3) return {};
    // извлекаем размеры сетки
    int nx = dims[0];
    int ny = dims[1];
    int nz = dims[2];
    
    std::vector<float> vertices;

    // если COORD или ZCORN отсутствуют, откатываемся на чистый параметрический бокс
    if (is_data_empty(coords, zcorn)) {
        std::cout << "[GridBuilder] COORD or ZCORN is empty. Using parametric dummy grid." << std::endl;
        generate_dummy_grid(nx, ny, nz, vertices);
        return vertices;
    }

    // TODO: Здесь подключаем полноценный перебор ячеек по ZCORN и COORD с вызовом get_pillar
    std::cout << "[GridBuilder] Building CPG mesh for grid: " << nx << " x " << ny << " x " << nz << std::endl;
    
    // пока для безопасности ответа набросим заглушку, чтобы код компилировался
    generate_dummy_grid(nx, ny, nz, vertices);
    
    return vertices;
}
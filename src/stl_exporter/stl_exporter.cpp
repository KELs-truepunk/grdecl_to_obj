#include "stl_exporter.hpp"
#include <fstream>
#include <cmath>
#include <iostream>
// структура для представления 3D-вектора
struct Vector3 {
    float x, y, z;
};

// функция для расчета нормали треугольника
static Vector3 compute_normal(const Vector3& p1, const Vector3& p2, const Vector3& p3) {
   // Вычисляем векторы двух сторон треугольника
    Vector3 u = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
    Vector3 v = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
    // Вычисляем векторное произведение u и v для получения нормали
    Vector3 n = {
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x
    };
    // нормализуем нормаль
    float len = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
    // Проверяем, что длина нормали не слишком мала, чтобы избежать деления на ноль
    if (len > 1e-6f) {
        n.x /= len; n.y /= len; n.z /= len;
    }
    return n;
}

bool StlExporter::export_to_stl(const std::string& filepath,
    std::vector<float>& vertices) {
    // На одну ячейку должны приходиться 8 вершин, по 3 координаты у каждой.
    if (vertices.empty() || vertices.size() % 24 != 0) {
        std::cerr << "[-] StlExporter: Invalid or empty vertices vector." << std::endl;
        return false;
    }
    // Открываем файл для записи в формате STL
    std::ofstream file(filepath);
    // Проверяем, что файл успешно открыт
    if (!file.is_open()) {
        std::cerr << "[-] StlExporter: Could not open file for writing: " << filepath << std::endl;
        return false;
    }

    file << "solid ReservoirModel\n";
    // Каждая грань гексаэдра станет двумя треугольниками в STL.
    int num_cells = vertices.size() / 24;

    // Индексы четырёх углов каждой из шести граней в порядке обхода.
    int faces[6][4] = {
        {0, 3, 2, 1}, // Верх
        {4, 5, 6, 7}, // Низ
        {0, 1, 5, 4}, // Перед
        {1, 2, 6, 5}, // Право
        {2, 3, 7, 6}, // Зад
        {3, 0, 4, 7}  // Лево
    };
    // Для каждой ячейки извлекаем грани и записываем их треугольники.
    for (int c = 0; c < num_cells; ++c) {
        const float* cell_v = &vertices[c * 24];
        // Проходим по каждой из 6 граней ячейки
        for (int f = 0; f < 6; ++f) {
            // извлекаем 4 вершины грани
            Vector3 pts[4];
            // заполняем массив pts координатами вершин грани
            for (int v = 0; v < 4; ++v) {
                int idx = faces[f][v] * 3;
                pts[v] = {cell_v[idx], cell_v[idx + 1], cell_v[idx + 2]};
            }

            // Диагональ делит четырёхугольник на два треугольника.
            int tri_indices[2][3] = {
                {0, 1, 2},
                {0, 2, 3}
            };
            // Вычисляем нормали и записываем треугольники в STL
            for (int t = 0; t < 2; ++t) {
                Vector3 p1 = pts[tri_indices[t][0]];
                Vector3 p2 = pts[tri_indices[t][1]];
                Vector3 p3 = pts[tri_indices[t][2]];

                Vector3 norm = compute_normal(p1, p2, p3);

                file << "  facet normal " << norm.x << " " << norm.y << " " << norm.z << "\n";
                file << "    outer loop\n";
                file << "      vertex " << p1.x << " " << p1.y << " " << p1.z << "\n";
                file << "      vertex " << p2.x << " " << p2.y << " " << p2.z << "\n";
                file << "      vertex " << p3.x << " " << p3.y << " " << p3.z << "\n";
                file << "    endloop\n";
                file << "  endfacet\n";
            }
        }
    }
    // Закрываем STL-модель и завершаем запись.
    file << "endsolid ReservoirModel\n";
    file.close();
    
    std::cout << "[+] Successfully exported " << num_cells << " cells to STL: " << filepath << std::endl;
    return true;
}
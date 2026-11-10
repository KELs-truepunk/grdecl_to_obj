#include "obj_exporter.hpp"
#include <iostream>
#include <fstream>

bool ObjExporter::export_optimized_obj(
    const std::string& filepath,
    const std::vector<float>& raw_vertices,
    const std::vector<int>& dims,
    bool add_base_plate,
    float base_thickness) 
{
    if (raw_vertices.empty() || dims.size() < 3) return false;

    int nx = dims[0], ny = dims[1], nz = dims[2];
    ObjMesh mesh;

    // Карта для уникальных вершин (чтобы не дублировать точки в OBJ)
    std::map<std::string, int> unique_vertices;
    
    auto get_vertex_index = [&](float x, float y, float z) {
        // Округляем до 4 знаков, чтобы избежать ошибок float
        std::string key = std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
        if (unique_vertices.find(key) == unique_vertices.end()) {
            int new_idx = mesh.vertices.size() / 3;
            unique_vertices[key] = new_idx;
            mesh.vertices.push_back(x);
            mesh.vertices.push_back(y);
            mesh.vertices.push_back(z);
        }
        return unique_vertices[key];
    };

    int num_cells = raw_vertices.size() / 24;
    
    auto has_neighbor = [&](int i, int j, int k, int di, int dj, int dk) {
        int ni = i + di, nj = j + dj, nk = k + dk;
        if (ni >= 0 && ni < nx && nj >= 0 && nj < ny && nk >= 0 && nk < nz) return true;
        return false;
    };

    // Проходим по всем ячейкам (порядок k, j, i как в GridBuilder)
    for (int k = 0; k < nz; ++k) {
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                int cell_idx = (k * ny + j) * nx + i;
                
                // Проверка: если ячейка вылетела за пределы массива (т.к. мы пропускали ACTNUM)
                // В сыром массиве вершин остались только активные ячейки. 
                // Это косяк передачи сырых вершин, но для простоты пока считаем, что сетка полная.
                if (cell_idx * 8 >= raw_vertices.size()) continue;

                int base_v = cell_idx * 8; 

                // Извлекаем 8 вершин текущей ячейки
                std::array<int, 8> v;
                for (int idx = 0; idx < 8; ++idx) {
                    v[idx] = get_vertex_index(
                        raw_vertices[base_v*3 + idx*3], 
                        raw_vertices[base_v*3 + idx*3 + 1], 
                        raw_vertices[base_v*3 + idx*3 + 2]
                    );
                }

                bool draw_bottom = !(add_base_plate && k == 0);

                // Верх (нет соседа сверху)
                if (!has_neighbor(i, j, k, 0, 0, 1)) {
                    mesh.faces.push_back({v[0], v[3], v[2]});
                    mesh.faces.push_back({v[0], v[2], v[1]});
                }
                // Низ (нет соседа снизу)
                if (draw_bottom && !has_neighbor(i, j, k, 0, 0, -1)) {
                    mesh.faces.push_back({v[4], v[5], v[6]});
                    mesh.faces.push_back({v[4], v[6], v[7]});
                }
                // Перед (нет соседа спереди)
                if (!has_neighbor(i, j, k, 0, -1, 0)) {
                    mesh.faces.push_back({v[0], v[1], v[5]});
                    mesh.faces.push_back({v[0], v[5], v[4]});
                }
                // Право (нет соседа справа)
                if (!has_neighbor(i, j, k, 1, 0, 0)) {
                    mesh.faces.push_back({v[1], v[2], v[6]});
                    mesh.faces.push_back({v[1], v[6], v[5]});
                }
                // Зад (нет соседа сзади)
                if (!has_neighbor(i, j, k, 0, 1, 0)) {
                    mesh.faces.push_back({v[2], v[3], v[7]});
                    mesh.faces.push_back({v[2], v[7], v[6]});
                }
                // Лево (нет соседа слева)
                if (!has_neighbor(i, j, k, -1, 0, 0)) {
                    mesh.faces.push_back({v[3], v[0], v[4]});
                    mesh.faces.push_back({v[3], v[4], v[7]});
                }
            }
        }
    }

    // --- ДОБАВЛЕНИЕ ПОДЛОЖКИ ---
    if (add_base_plate) {
        float min_x = 1e9, max_x = -1e9, min_y = 1e9, max_y = -1e9, min_z = 1e9;
        for (size_t i = 0; i < mesh.vertices.size(); i += 3) {
            min_x = std::min(min_x, mesh.vertices[i]);     max_x = std::max(max_x, mesh.vertices[i]);
            min_y = std::min(min_y, mesh.vertices[i+1]);   max_y = std::max(max_y, mesh.vertices[i+1]);
            min_z = std::min(min_z, mesh.vertices[i+2]);
        }
        
        int p0 = get_vertex_index(min_x, min_y, min_z);
        int p1 = get_vertex_index(max_x, min_y, min_z);
        int p2 = get_vertex_index(max_x, max_y, min_z);
        int p3 = get_vertex_index(min_x, max_y, min_z);
        
        int b0 = get_vertex_index(min_x, min_y, min_z - base_thickness);
        int b1 = get_vertex_index(max_x, min_y, min_z - base_thickness);
        int b2 = get_vertex_index(max_x, max_y, min_z - base_thickness);
        int b3 = get_vertex_index(min_x, max_y, min_z - base_thickness);

        // Верх подложки
        mesh.faces.push_back({p0, p1, p2}); mesh.faces.push_back({p0, p2, p3});
        // Низ подложки
        mesh.faces.push_back({b0, b3, b2}); mesh.faces.push_back({b0, b2, b1});
        // Стенки
        mesh.faces.push_back({p0, b0, b1}); mesh.faces.push_back({p0, b1, p1});
        mesh.faces.push_back({p1, b1, b2}); mesh.faces.push_back({p1, b2, p2});
        mesh.faces.push_back({p2, b2, b3}); mesh.faces.push_back({p2, b3, p3});
        mesh.faces.push_back({p3, b3, b0}); mesh.faces.push_back({p3, b0, p0});
    }

    // Запись в файл
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    file << "# Optimized Mesh Export (NOVATEK MVP)\n";
    for (size_t i = 0; i < mesh.vertices.size(); i += 3) {
        file << "v " << mesh.vertices[i] << " " << mesh.vertices[i+1] << " " << mesh.vertices[i+2] << "\n";
    }
    for (const auto& f : mesh.faces) {
        file << "f " << f.v0 + 1 << " " << f.v1 + 1 << " " << f.v2 + 1 << "\n";
    }
    
    file.close();
    std::cout << "[+] Exported optimized OBJ: " << mesh.vertices.size()/3 << " verts, " << mesh.faces.size() << " faces." << std::endl;
    return true;
}
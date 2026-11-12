#include "obj_exporter.hpp"
#include <iostream>
#include <fstream>

bool ObjExporter::export_optimized_obj(
    const std::string& filepath,
    const std::vector<float>& raw_vertices,
    const std::vector<int>& dims,
    const std::vector<float>& actnum, 
    bool add_base_plate,
    float base_thickness) 
{
    // Без координат или размеров сетки невозможно сопоставить вершины ячейкам.
    if (raw_vertices.empty() || dims.size() < 3) return false;

    int nx = dims[0], ny = dims[1], nz = dims[2];
    ObjMesh mesh;

    // Соседние ячейки делят общие углы; сохраняем каждую уникальную координату один раз.
    std::map<std::string, int> unique_vertices;
    auto get_vertex_index = [&](float x, float y, float z) {
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

    // Вспомогательная лямбда: активна ли ячейка?
    auto is_active = [&](int i, int j, int k) {
        if (i < 0 || i >= nx || j < 0 || j >= ny || k < 0 || k >= nz) return false;
        int idx = (k * ny + j) * nx + i;
        if (!actnum.empty() && idx < actnum.size()) {
            return actnum[idx] >= 0.5f;
        }
        return true; // Если actnum пустой, считаем все ячейки активными
    };

    // В raw_vertices находятся только активные ячейки, поэтому ведём отдельный индекс.
    int active_vertex_ptr = 0;

    for (int k = 0; k < nz; ++k) {
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                
                if (!is_active(i, j, k)) continue; // Пропускаем мертвые ячейки

                int base_v = active_vertex_ptr * 24; 
                active_vertex_ptr++;

                // Проверка на всякий случай, чтобы не вылететь
                if (base_v + 23 >= raw_vertices.size()) break;

                std::array<int, 8> v;
                for (int idx = 0; idx < 8; ++idx) {
                    v[idx] = get_vertex_index(
                        raw_vertices[base_v + idx*3], 
                        raw_vertices[base_v + idx*3 + 1], 
                        raw_vertices[base_v + idx*3 + 2]
                    );
                }

                // Проверяем соседей (используем лямбду is_active)
                auto has_active_neighbor = [&](int di, int dj, int dk) {
                    return is_active(i + di, j + dj, k + dk);
                };

                // При наличии подложки нижние грани первого слоя будут закрыты ею.
                bool draw_bottom = !(add_base_plate && k == 0);

                // Верх
                if (!has_active_neighbor(0, 0, 1)) {
                    mesh.faces.push_back({v[0], v[3], v[2]});
                    mesh.faces.push_back({v[0], v[2], v[1]});
                }
                // Низ
                if (draw_bottom && !has_active_neighbor(0, 0, -1)) {
                    mesh.faces.push_back({v[4], v[5], v[6]});
                    mesh.faces.push_back({v[4], v[6], v[7]});
                }
                // Перед
                if (!has_active_neighbor(0, -1, 0)) {
                    mesh.faces.push_back({v[0], v[1], v[5]});
                    mesh.faces.push_back({v[0], v[5], v[4]});
                }
                // Право
                if (!has_active_neighbor(1, 0, 0)) {
                    mesh.faces.push_back({v[1], v[2], v[6]});
                    mesh.faces.push_back({v[1], v[6], v[5]});
                }
                // Зад
                if (!has_active_neighbor(0, 1, 0)) {
                    mesh.faces.push_back({v[2], v[3], v[7]});
                    mesh.faces.push_back({v[2], v[7], v[6]});
                }
                // Лево
                if (!has_active_neighbor(-1, 0, 0)) {
                    mesh.faces.push_back({v[3], v[0], v[4]});
                    mesh.faces.push_back({v[3], v[4], v[7]});
                }
            }
        }
    }

    // При необходимости добавляем под сеткой прямоугольную подложку заданной толщины.
    if (add_base_plate && !mesh.vertices.empty()) {
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

        mesh.faces.push_back({p0, p1, p2}); mesh.faces.push_back({p0, p2, p3});
        mesh.faces.push_back({b0, b3, b2}); mesh.faces.push_back({b0, b2, b1});
        mesh.faces.push_back({p0, b0, b1}); mesh.faces.push_back({p0, b1, p1});
        mesh.faces.push_back({p1, b1, b2}); mesh.faces.push_back({p1, b2, p2});
        mesh.faces.push_back({p2, b2, b3}); mesh.faces.push_back({p2, b3, p3});
        mesh.faces.push_back({p3, b3, b0}); mesh.faces.push_back({p3, b0, p0});
    }

    // OBJ хранит вершины отдельно, а грани ссылаются на них с индексацией от единицы.
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
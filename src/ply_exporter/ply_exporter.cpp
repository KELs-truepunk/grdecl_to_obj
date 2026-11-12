#include "ply_exporter.hpp"
#include <fstream>
#include <iostream>
#include <string>

bool PlyExporter::export_to_ply(const std::string& filepath, const std::vector<float>& vertices, bool binary) {
    // Входной массив должен содержать как минимум одну ячейку из восьми вершин.
    if (vertices.empty() || vertices.size() % 24 != 0) {
        std::cerr << "[-] PlyExporter: Invalid or empty vertices vector." << std::endl;
        return false;
    }
    // Файл открывается в бинарном режиме, чтобы поддержать и текстовую, и двоичную запись.
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()){
        std::cerr << "[-] PlyExporter: Could not open file: " << filepath << std::endl;
        return false;
    }
    const int num_cells = static_cast<int>(vertices.size() / 24) ;
    const int num_verts = static_cast<int>(vertices.size() / 3);
    const int num_faces = num_cells * 6;
    
    // В заголовке описаны формат файла, вершины и четыре индекса каждой грани.
    file << "ply \n";
    file << (binary ? "format binary_little_endian 1.0\n" : "format ascii 1.0\n");
    file << "element vertex " << num_verts << "\n";
    file << "property float x\n";
    file << "property float y\n";
    file << "property float z\n";
    file << "element face " << num_faces << "\n";
    file << "property list uchar int vertex_indices\n";
    file << "end_header\n";

    // Координаты точек следуют сразу за заголовком.
    if (binary) {
        for (float v : vertices) {
            file.write(reinterpret_cast<const char*>(&v), sizeof(float));
        }
    } else {
        for (size_t i = 0; i < vertices.size(); i += 3) {
            file << vertices[i] << " "
                 << vertices[i + 1] << " "
                 << vertices[i + 2] << "\n";
        }
    }
    static const int faces[6][4] = {
        {0, 3, 2, 1},  // вверх
        {4, 5, 6, 7},  // низ
        {0, 1, 5, 4},  // перед
        {1, 2, 6, 5},  // право
        {2, 3, 7, 6},  // зад
        {3, 0, 4, 7}   // лево
    };

    // Каждую грань четырёхугольной ячейки записываем четырьмя индексами вершин.
    for (int c = 0; c < num_cells; ++c) {
        const int base = c * 8;
        for (int f = 0; f < 6; ++f) {
            if (binary) {
                unsigned char count = 4;
                file.write(reinterpret_cast<const char*>(&count), 1);
                for (int v = 0; v < 4; ++v) {
                    int idx = base + faces[f][v];
                    file.write(reinterpret_cast<const char*>(&idx), sizeof(int));
                }
            } else {
                file << "4 "
                     << (base + faces[f][0]) << " "
                     << (base + faces[f][1]) << " "
                     << (base + faces[f][2]) << " "
                     << (base + faces[f][3]) << "\n";
            }
        }
    }

    file.close();
    std::cout << "[+] PLY: " << num_cells << " cells -> " << filepath
              << (binary ? " (binary)" : " (ascii)") << std::endl;
    return true;
}

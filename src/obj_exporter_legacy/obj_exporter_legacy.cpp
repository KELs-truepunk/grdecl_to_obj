#include "obj_exporter_legacy.hpp"
#include <fstream>
#include <iostream>

bool ObjExporterLegacy::export_to_obj(const std::string& filepath, const std::vector<float>& vertices) {
    // На каждую ячейку приходится восемь вершин с тремя координатами.
    if (vertices.empty() || vertices.size() % 24 != 0) {
        std::cerr << "[-] ObjExporter: Invalid or empty vertices vector." << std::endl;
        return false;
    }
    // Открываем файл для записи
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[-] ObjExporter: Could not open file for writing: " << filepath << std::endl;
        return false;
    }

    file << "# Exported from GRDECL C++ Backend (NOVATEK Project)\n";
    // Вычисляем количество ячеек в сетке
    int num_cells = vertices.size() / 24; // 8 вершин по 3 координаты = 24 числа на ячейку

    // Записываем все вершины
    for (size_t i = 0; i < vertices.size(); i += 3) {
        // Формат OBJ: v x y z
        file << "v " << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << "\n";
    }

    // Локальные индексы шести четырёхугольных граней в порядке обхода вершин ячейки.
    int faces[6][4] = {
        {0, 3, 2, 1}, // Верх
        {4, 5, 6, 7}, // Низ
        {0, 1, 5, 4}, // Перед
        {1, 2, 6, 5}, // Право
        {2, 3, 7, 6}, // Зад
        {3, 0, 4, 7}  // Лево
    };

    // Для каждой ячейки переводим локальные индексы в общие индексы OBJ.
    for (int c = 0; c < num_cells; ++c) {
        int base_v_idx = c * 8 + 1; // OBJ индексация начинается с 1
        // Записываем 6 граней ячейки
        for (int f = 0; f < 6; ++f) {
            file << "f";
            // Записываем индексы вершин грани (4 вершины на грань)
            for (int v = 0; v < 4; ++v) {
                file << " " << (base_v_idx + faces[f][v]);
            }
            // Записываем новую строку после каждой грани
            file << "\n";
        }
    }
    // Закрываем файл и выводим сообщение об успешном экспорте
    file.close();
    std::cout << "[+] Successfully exported " << num_cells << " cells to " << filepath << "[cite: 1]" << std::endl;
    return true;
}
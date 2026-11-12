#include "vtk_exporter.hpp"
#include <fstream>
#include <iostream>

bool VtkExporter::export_to_vtk(const std::string& filepath,
                                const std::vector<float>& vertices) {
    // Размер массива должен соответствовать восьми вершинам на ячейку.
    if (vertices.empty() || vertices.size() % 24 != 0) {
        std::cerr << "[-] VtkExporter: Invalid vertices." << std::endl;
        return false;
    }
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[-] VtkExporter: Cannot open: " << filepath << std::endl;
        return false;
    }

    // В этом формате каждая ячейка использует собственные точки.
    const int num_cells  = static_cast<int>(vertices.size() / 24);
    const int num_points = static_cast<int>(vertices.size() / 3);

    // Заголовок объявляет текстовый файл с неструктурированной сеткой.
    file << "# vtk DataFile Version 3.0\n";
    file << "GRDECL Export - NOVATEK Project\n";
    file << "ASCII\n";
    file << "DATASET UNSTRUCTURED_GRID\n";

    // Координаты записываются тройками: X, Y, Z.
    file << "POINTS " << num_points << " float\n";
    for (size_t i = 0; i < vertices.size(); i += 3) {
        file << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << "\n";
    }

    // Каждая запись содержит число вершин ячейки и индексы этих вершин.
    file << "CELLS " << num_cells << " " << (num_cells * 9) << "\n";
    for (int c = 0; c < num_cells; ++c) {
        const int base = c * 8;
        file << "8";
        for (int v = 0; v < 8; ++v) file << " " << (base + v);
        file << "\n";
    }

    // Тип 12 в VTK обозначает гексаэдр.
    file << "CELL_TYPES " << num_cells << "\n";
    for (int c = 0; c < num_cells; ++c) file << "12\n";

    file.close();
    std::cout << "[+] VTK: " << num_cells << " hex cells -> " << filepath << std::endl;
    return true;
}
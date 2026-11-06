#include "vtu_exporter.hpp"
#include <fstream>
#include <iostream>

bool VtuExporter::export_to_vtu(const std::string& filepath,
                                const std::vector<float>& vertices) {
    if (vertices.empty() || vertices.size() % 24 != 0) {
        std::cerr << "[-] VtuExporter: Invalid vertices." << std::endl;
        return false;
    }
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[-] VtuExporter: Cannot open: " << filepath << std::endl;
        return false;
    }

    const int num_cells  = static_cast<int>(vertices.size() / 24);
    const int num_points = static_cast<int>(vertices.size() / 3);

    file << "<?xml version=\"1.0\"?>\n";
    file << "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\">\n";
    file << "  <UnstructuredGrid>\n";
    file << "    <Piece NumberOfPoints=\"" << num_points
         << "\" NumberOfCells=\"" << num_cells << "\">\n";

    // Точки
    file << "      <Points>\n";
    file << "        <DataArray type=\"Float32\" NumberOfComponents=\"3\" format=\"ascii\">\n";
    for (size_t i = 0; i < vertices.size(); i += 3) {
        file << "          " << vertices[i] << " "
             << vertices[i + 1] << " " << vertices[i + 2] << "\n";
    }
    file << "        </DataArray>\n";
    file << "      </Points>\n";

    // Ячейки
    file << "      <Cells>\n";

    file << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";
    for (int c = 0; c < num_cells; ++c) {
        file << "          ";
        const int base = c * 8;
        for (int v = 0; v < 8; ++v) {
            if (v) file << " ";
            file << (base + v);
        }
        file << "\n";
    }
    file << "        </DataArray>\n";

    file << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";
    for (int c = 0; c < num_cells; ++c) {
        file << "          " << ((c + 1) * 8) << "\n";
    }
    file << "        </DataArray>\n";

    file << "        <DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";
    for (int c = 0; c < num_cells; ++c) {
        file << "          12\n"; // VTK_HEXAHEDRON
    }
    file << "        </DataArray>\n";

    file << "      </Cells>\n";
    file << "    </Piece>\n";
    file << "  </UnstructuredGrid>\n";
    file << "</VTKFile>\n";

    file.close();
    std::cout << "[+] VTU: " << num_cells << " cells -> " << filepath << std::endl;
    return true;
}
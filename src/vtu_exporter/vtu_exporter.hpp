#pragma once
#include <string>
#include <vector>

// Сохраняет сетку в формате VTU, который можно открыть в ParaView и других VTK-программах.
class VtuExporter {
public:
    // Вершины передаются подряд: по восемь точек с координатами X, Y, Z на ячейку.
    static bool export_to_vtu(const std::string& filepath,
                              const std::vector<float>& vertices);
};
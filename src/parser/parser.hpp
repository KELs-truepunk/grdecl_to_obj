#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Parser {
    public:
    //explict - запрет на неявное преобразование(сак май эс, пайтон)
    //сам вызов парсера - его конструктор с указанием пути до файла,который нужно распарсить
    explicit Parser(const std::string file_path);

    bool load(); //метод загрузки файла в память
    
    std::vector<int> get_specgrid() const; /*возвращет вектор с размерами сетки {nx, ny, nz} или пустой вектор
    если не удалось прочитать файл*/

    std::vector<float> parse_keyword(const std::string& keyword) const; /*возвращает массив чисел для любого ключевого слова
    с учетом RLE-декомпрессии*/
    private:
        std::string file_path;
        std::string file_content;
    
        bool read_file_and_resolve_includes(const std::string& cur_path,
            std::string& out_content);
};
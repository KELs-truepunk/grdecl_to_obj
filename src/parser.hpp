#pragma once

#include <string>
#include <vector>
#include <unordered_map>

class Parser {
    public:
    //explict - запрет на неявное преобразование(сак май эс, пайтон)
    explicit Parser(const std::string file_path);

    bool parse_specgrid(int &nx, int& ny, int& nz); //главный метод: парсим файл и изменяем значения nx, ny, nz по ссылке
    std::vector<float> parse_keyword_data(const std::string& keyword); //метод чтения массивов с RLE-декомпрессией 

    private:
    std::string file_path;
    std::string file_content;

    int read_file(); //метод чтения файла в строку
};
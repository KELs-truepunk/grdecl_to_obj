#include "parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
//пространство имен для работы с файлами
namespace fs = std::filesystem;

/*запуск объекта парсера с указанием пути к файлу
std::move() здесь перемещает объект, чтобы избежать лишнего копирования
*/
Parser::Parser(const std::string path) : file_path(std::move(path)) {}

bool Parser::load(){
    file_content.clear();
    // Загружаем основной файл вместе с содержимым всех подключённых INCLUDE-файлов.
    return read_file_and_resolve_includes(file_path, file_content);
}

// Рекурсивная функция для чтения файла и обработки INCLUDE
bool Parser::read_file_and_resolve_includes(const std::string& current_path, std::string& out_content) {
    std::ifstream file(current_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << current_path << std::endl;
        return false;
    }

    // Получаем директорию текущего файла, чтобы знать, где искать инклуды
    fs::path current_dir = fs::path(current_path).parent_path();
    std::string line;
    while (std::getline(file, line)) {
        // Ищем слово INCLUDE
        size_t include_pos = line.find("INCLUDE");
        
        if (include_pos != std::string::npos) {
            // Если нашли INCLUDE, читаем следующую строку, где будет имя файла
            std::string include_line;
            if (std::getline(file, include_line)) {
                // Ищем кавычки
                size_t start_quote = include_line.find_first_of("'\"");
                size_t end_quote = include_line.find_last_of("'\"");
                
                if (start_quote != std::string::npos && end_quote != std::string::npos && end_quote > start_quote) {
                    // Вырезаем имя файла без кавычек
                    std::string inc_filename = include_line.substr(start_quote + 1, end_quote - start_quote - 1);
                    
                    // Формируем полный путь (папка главного файла + имя инклуда)
                    fs::path inc_path = current_dir / inc_filename;
                    
                    // РЕКУРСИЯ: Читаем этот инклуд и дописываем в общий out_content
                    if (!read_file_and_resolve_includes(inc_path.string(), out_content)) {
                        std::cerr << "Warning: Failed to include file: " << inc_path << std::endl;
                    }
                }
            }
        } else {
            // Если это не инклуд, просто добавляем строку в общий текст
            out_content += line + "\n";
        }
    }
    return true;
}

// Замени старые функции на эти:

std::vector<int> Parser::get_specgrid() const {
    // Ищем SPECGRID как отдельное слово
    size_t pos = file_content.find("SPECGRID");
    
    while (pos != std::string::npos) {
        // Проверяем, что до и после идут пробелы или переносы строк
        bool valid_before = (pos == 0 || isspace(file_content[pos - 1]));
        bool valid_after = (pos + 8 >= file_content.size() || isspace(file_content[pos + 8]));
        
        if (valid_before && valid_after) break; // Нашли нужное!
        pos = file_content.find("SPECGRID", pos + 1); // Ищем дальше
    }

    if (pos == std::string::npos) return {}; 
    
    // После SPECGRID читаем три размера до завершающего слэша.
    std::stringstream ss(file_content.substr(pos + 8)); 
    std::string token;
    std::vector<int> dims;
    
    while (ss >> token) {
        if (token == "/") break;
        if (token.rfind("--", 0) == 0) continue; 

        try {
            dims.push_back(std::stoi(token)); 
            if (dims.size() == 3) break; 
        }   
        catch(...){}
    }
    return dims; 
}

std::vector<float> Parser::parse_keyword(const std::string& keyword) const {
    std::vector<float> data;
    
    // Ищем ключевое слово, но проверяем, что это ОТДЕЛЬНОЕ слово
    size_t pos = file_content.find(keyword);
    while (pos != std::string::npos) {
        bool valid_before = (pos == 0 || isspace(file_content[pos - 1]));
        bool valid_after = (pos + keyword.length() >= file_content.size() || isspace(file_content[pos + keyword.length()]));
        
        if (valid_before && valid_after) break;
        pos = file_content.find(keyword, pos + 1);
    }

    if (pos == std::string::npos) return data; 
    
    // Читаем значения до слэша; запись вида «число*значение» раскрываем в отдельные элементы.
    std::stringstream ss(file_content.substr(pos + keyword.length())); 
    std::string token;
    while (ss >> token){
        if(token == "/") break; 
        
        if(token.rfind("--", 0) == 0){ 
            std::string dummy;
            std::getline(ss, dummy); 
            continue;
        }
        
        size_t star_pos = token.find('*');
        if(star_pos != std::string::npos){
            try {
                int cnt = std::stoi(token.substr(0, star_pos)); 
                float val = std::stof(token.substr(star_pos + 1)); 
                data.insert(data.end(), cnt, val); 
            } catch(...) {}
        }else{
            try{
                data.push_back(std::stof(token)); 
            }
            catch(...){
                // Если встретили мусор (буквы), просто идем дальше
                continue; 
            }
        }
    }
    return data;
}
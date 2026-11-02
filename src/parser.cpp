#include "parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
/*запуск объекта парсера с указанием пути к файлу
std::move() здесь перемещает объект, чтобы избежать лишнего копирования
*/
Parser::Parser(const std::string path) : file_path(std::move(path)) {}

bool Parser::load(){
    std::ifstream file(file_path, std::ios::binary); //открытие файла в бинарном виде
    //ошибка открытия файла
    if (!file.is_open()) {
        //вывод ошибки в stderr поток(терминал)
        std::cerr << "Error: Could not open file " << file_path << std::endl;
        return false;
    }
    file.seekg(0, std::ios::end); //перемещаем указатель в конец файла
    //получаем размер файла
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg); //перемещаем указатель в начало файла
    
    file_content.resize(file_size); //выделяем память под содержимое файла
    file.read(&file_content[0], file_size); //читаем содержимое файла
    file.close(); //закрываем файл
    return true;
}


cmake_minimum_required(VERSION 3.16)
project(grdecl_viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON) # Автоматическая обработка сигналов/слотов Qt

# Ищем библиотеки
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(VTK REQUIRED COMPONENTS 
    CommonCore 
    CommonDataModel 
    RenderingCore 
    RenderingOpenGL2 
    InteractionStyle 
    GUISupportQt
)

# Подключаем наш src
include_directories(${CMAKE_SOURCE_DIR}/src)

# Собираем все cpp файлы из src
file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(grdecl_viewer 
    main.cpp 
    ${SRC_FILES}
)

# Линкуем Qt и VTK
target_link_libraries(grdecl_viewer PRIVATE 
    Qt6::Widgets 
    ${VTK_LIBRARIES}
)

# Это нужно для того, чтобы VTK понимал Qt (в новых версиях VTK)
vtk_module_autoinit(
    TARGETS grdecl_viewer
    MODULES ${VTK_LIBRARIES}
)
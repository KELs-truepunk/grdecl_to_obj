#pragma once
#include <QMainWindow>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <QVTKOpenGLNativeWidget.h>
#include <QDoubleSpinBox>
#include <QLabel>
#include <vector>

// Главное окно: загрузка модели и параметров, настройка масштаба, просмотр и экспорт.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

private slots:
    // Действия, вызываемые кнопками интерфейса или изменением масштаба.
    void open_file_dialog();
    void refresh_3d_view();
    void open_parameters_dialog();
    void export_obj(); 

private:
    void setup_ui();
    void show_welcome_box();
    bool has_fluids = false; // Показывает, загружены ли значения для раскраски ячеек.
    std::vector<int> current_dims; // Размеры сетки по осям X, Y и Z.
    std::vector<float> current_coords; // Координаты концов пилларов из COORD.
    std::vector<float> current_zcorn; // Глубины углов ячеек из ZCORN.
    std::vector<float> current_actnum; // Признак активности каждой ячейки из ACTNUM.
    std::vector<float> current_fluids; // Значения свойства, используемые для раскраски.

    // Виджеты и компоненты VTK, которые обновляются при перестроении сцены.
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkRenderer> renderer;
    QDoubleSpinBox* zScaleSpin;
    QLabel* statusLabel;
};
#pragma once
#include <QMainWindow>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <QVTKOpenGLNativeWidget.h>
#include <QDoubleSpinBox>
#include <QLabel>
#include <vector>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() = default;

private slots:
    void open_file_dialog();
    void refresh_3d_view();
    void export_obj(); 

private:
    void setup_ui();
    void show_welcome_box();

    std::vector<int> current_dims;
    std::vector<float> current_coords;
    std::vector<float> current_zcorn;
    std::vector<float> current_actnum;

    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkRenderer> renderer;
    QDoubleSpinBox* zScaleSpin;
    QLabel* statusLabel;
};
#include "main_window.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>

// Подключаем ядро!
#include "parser/parser.hpp"
#include "grid_builder/grid_builder.hpp"
#include "obj_exporter/obj_exporter.hpp" 

// VTK
#include <vtkProperty.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkHexahedron.h>
#include <vtkCellArray.h>
#include <vtkDataSetMapper.h>
#include <vtkFloatArray.h>               
#include <vtkColorTransferFunction.h>    
#include <vtkCellData.h>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("GRDECL Viewer C++ / Qt6 / VTK");
    resize(1200, 800);
    setup_ui();
    show_welcome_box();
}

void MainWindow::setup_ui() {
    QWidget* centralWidget = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    QWidget* sidebar = new QWidget(centralWidget);
    sidebar->setFixedWidth(280);
    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);

    QPushButton* btnLoad = new QPushButton("Открыть .GRDECL файл", sidebar);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::open_file_dialog);
    sidebarLayout->addWidget(btnLoad);

    QPushButton* btnLoadParams = new QPushButton("Загрузить параметры (FLUIDS)", sidebar);
    connect(btnLoadParams, &QPushButton::clicked, this, &MainWindow::open_parameters_dialog);
    sidebarLayout->addWidget(btnLoadParams);

    sidebarLayout->addWidget(new QLabel("Z-Scale:", sidebar));
    zScaleSpin = new QDoubleSpinBox(sidebar);
    zScaleSpin->setRange(1.0, 50.0);
    zScaleSpin->setSingleStep(1.0);
    zScaleSpin->setValue(1.0);
    connect(zScaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::refresh_3d_view);
    sidebarLayout->addWidget(zScaleSpin);

    QPushButton* btnExport = new QPushButton("Экспорт в .OBJ", sidebar);
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::export_obj);
    sidebarLayout->addWidget(btnExport);

    statusLabel = new QLabel("Модель не загружена", sidebar);
    statusLabel->setWordWrap(true);
    sidebarLayout->addWidget(statusLabel);

    sidebarLayout->addStretch();
    mainLayout->addWidget(sidebar);

    vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    vtkWidget->setRenderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New());
    
    renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkWidget->renderWindow()->AddRenderer(renderer);
    
    renderer->SetBackground(0.1, 0.2, 0.4); 
    renderer->SetBackground2(0.8, 0.9, 1.0);
    renderer->GradientBackgroundOn();

    mainLayout->addWidget(vtkWidget, 1);
    setCentralWidget(centralWidget);
}

void MainWindow::show_welcome_box() {
    vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cubeSource->GetOutputPort());
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    renderer->AddActor(actor);
    renderer->ResetCamera();
}

void MainWindow::open_file_dialog() {
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть модель пласта", "", "GRDECL Files (*.GRDECL *.grdecl);;All Files (*)");
    
    if (filePath.isEmpty()) return;

    try {
        Parser parser(filePath.toStdString());
        if (!parser.load()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл.");
            return;
        }

        current_dims = parser.get_specgrid();
        if (current_dims.size() != 3) {
            QMessageBox::warning(this, "Внимание", "SPECGRID не найден или поврежден.");
            return;
        }

        current_coords = parser.parse_keyword("COORD");
        current_zcorn = parser.parse_keyword("ZCORN");
        current_actnum = parser.parse_keyword("ACTNUM");
        //current_fluids = parser.parse_keyword("FLUIDS"); 

        refresh_3d_view();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", QString("Ошибка парсинга: %1").arg(e.what()));
    }
}

void MainWindow::refresh_3d_view() {
    if (current_dims.empty()) return;

    float zVal = zScaleSpin->value();
    
    // Строим сетку (теперь возвращается MeshData)
    auto mesh = GridBuilder::build_mesh_grid(current_dims, current_coords, current_zcorn, current_actnum, current_fluids, zVal);
    
    if (mesh.vertices.empty()) {
        QMessageBox::warning(this, "Внимание", "Сетка пуста.");
        return;
    }

    size_t num_cells = mesh.vertices.size() / 24;

    // --- СОЗДАНИЕ VTK GRID ---
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(mesh.vertices.size() / 3);

    for (size_t i = 0; i < mesh.vertices.size(); i += 3) {
        points->SetPoint(i / 3, mesh.vertices[i], mesh.vertices[i + 1], mesh.vertices[i + 2]);
    }

    vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->SetPoints(points);

    for (size_t c = 0; c < num_cells; ++c) {
        vtkSmartPointer<vtkHexahedron> hex = vtkSmartPointer<vtkHexahedron>::New();
        size_t base = c * 8;
        for (int v = 0; v < 8; ++v) {
            hex->GetPointIds()->SetId(v, base + v);
        }
        grid->InsertNextCell(hex->GetCellType(), hex->GetPointIds());
    }

    // --- ПРИКРУЧИВАЕМ ЦВЕТА (SCALARS) ---
    vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
    scalars->SetName("Fluids");
    for (float val : mesh.scalars) {
        scalars->InsertNextValue(val);
    }
    grid->GetCellData()->SetScalars(scalars);

    // Маппер и актор
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(grid);
    
    if (has_fluids && !mesh.scalars.empty()) {
        // Включаем раскраску, если параметры загружены
        mapper->ScalarVisibilityOn(); 
        mapper->SetScalarModeToUseCellData();
        mapper->SetColorModeToMapScalars();

        // Настраиваем палитру (LUT)
        vtkSmartPointer<vtkColorTransferFunction> lut = vtkSmartPointer<vtkColorTransferFunction>::New();
        lut->AddRGBPoint(0.0, 0.8, 0.8, 0.8); // 0 = Светло-серый (Вода/Пустая порода)
        lut->AddRGBPoint(5.0, 0.9, 0.4, 0.1); // 5 = Оранжевый (Нефть/Коллектор)
        mapper->SetLookupTable(lut);
    } else {
        // Если параметров нет, отключаем раскраску (рисуем базовым цветом)
        mapper->ScalarVisibilityOff();
    }

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    
    if (!has_fluids) {
        actor->GetProperty()->SetColor(0.5, 0.8, 1.0); // Светло-голубой
    }
    
    // Оставляем контуры для красоты
    actor->GetProperty()->EdgeVisibilityOn();
    actor->GetProperty()->SetEdgeColor(0.2, 0.2, 0.2);
    actor->GetProperty()->SetLineWidth(0.5);

    renderer->RemoveAllViewProps();
    renderer->AddActor(actor);
    renderer->ResetCamera();

    statusLabel->setText(QString("Grid: %1x%2x%3\nCells: %4\nZ-Scale: %5")
        .arg(current_dims[0]).arg(current_dims[1]).arg(current_dims[2])
        .arg(num_cells).arg(zVal));
        
    vtkWidget->renderWindow()->Render();
}

void MainWindow::open_parameters_dialog() {
    if (current_dims.empty()) {
        QMessageBox::warning(this, "Предупреждение", "Сначала откройте основной GRDECL файл!");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, "Открыть файл параметров", "", "GRDECL Files (*.GRDECL *.grdecl);;All Files (*)");
    if (filePath.isEmpty()) return;

    try {
        Parser paramParser(filePath.toStdString());
        if (!paramParser.load()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл параметров.");
            return;
        }

        // Пытаемся найти FLUIDS или PORO (или любое другое свойство)
        current_fluids = paramParser.parse_keyword("FLUIDS");
        if (current_fluids.empty()) {
            current_fluids = paramParser.parse_keyword("PORO");
        }
        
        if (current_fluids.empty()) {
            QMessageBox::warning(this, "Ошибка", "В файле не найдены известные ключевые слова (FLUIDS/PORO).");
            return;
        }

        has_fluids = true;
        refresh_3d_view(); // Перерисовываем с новыми цветами
        QMessageBox::information(this, "Успех", "Параметры загружены и применены!");
        
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", QString("Ошибка парсинга параметров: %1").arg(e.what()));
    }
}
void MainWindow::export_obj() {
    if (current_dims.empty()) {
        QMessageBox::warning(this, "Предупреждение", "Сначала откройте файл!");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(this, "Сохранить как OBJ", "", "Wavefront OBJ (*.obj);;All Files (*)");
    if (filePath.isEmpty()) return;

    try {
        float zVal = zScaleSpin->value();
        
        // Считаем данные (теперь возвращается MeshData)
        auto mesh = GridBuilder::build_mesh_grid(current_dims, current_coords, current_zcorn, current_actnum, current_fluids, zVal);

        if (mesh.vertices.empty()) {
            QMessageBox::warning(this, "Ошибка", "Нет данных для экспорта.");
            return;
        }

        bool success = ObjExporter::export_optimized_obj(
            filePath.toStdString(), 
            mesh.vertices, // <-- Передаем только vertices
            current_dims, 
            current_actnum,
            true,   
            10.0f   
        );

        if (success) {
            QMessageBox::information(this, "Успех", "Модель успешно сохранена!");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось записать файл.");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", QString("Ошибка экспорта: %1").arg(e.what()));
    }
}
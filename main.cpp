#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <iostream>

// VTK
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

// QVTK
#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char** argv) {
    // Обязательно для работы VTK внутри Qt
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("GRDECL Viewer C++ / Qt6 / VTK");
    mainWindow.resize(1200, 800);

    QWidget* centralWidget = new QWidget(&mainWindow);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

    // --- Левая панель ---
    QWidget* sidebar = new QWidget(centralWidget);
    sidebar->setFixedWidth(250);
    QVBoxLayout* sidebarLayout = new QVBoxLayout(sidebar);
    
    QLabel* title = new QLabel("Управление", sidebar);
    title->setStyleSheet("font-weight: bold; font-size: 14px;");
    sidebarLayout->addWidget(title);

    QPushButton* btnLoad = new QPushButton("Загрузить GRDECL", sidebar);
    sidebarLayout->addWidget(btnLoad);
    
    QPushButton* btnExport = new QPushButton("Экспорт в OBJ", sidebar);
    sidebarLayout->addWidget(btnExport);
    
    sidebarLayout->addStretch();
    mainLayout->addWidget(sidebar);

    // --- 3D Окно VTK ---
    QVTKOpenGLNativeWidget* vtkWidget = new QVTKOpenGLNativeWidget(centralWidget);
    vtkWidget->setRenderWindow(vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New());
    
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkWidget->renderWindow()->AddRenderer(renderer);
    
    // Красивый градиентный фон
    renderer->SetBackground(0.1, 0.2, 0.4); 
    renderer->SetBackground2(0.8, 0.9, 1.0);
    renderer->GradientBackgroundOn();

    mainLayout->addWidget(vtkWidget, 1);
    mainWindow.setCentralWidget(centralWidget);

    // --- Тестовый куб (заглушка) ---
    vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(cubeSource->GetOutputPort());
    
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    renderer->AddActor(actor);

    renderer->ResetCamera();

    // Заглушка для кнопки
    QObject::connect(btnLoad, &QPushButton::clicked, [&]() {
        std::cout << "Кнопка нажата! Скоро здесь будет загрузка файла." << std::endl;
    });

    mainWindow.show();
    return app.exec();
}
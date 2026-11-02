import sys
import os
from PyQt6.QtWidgets import QApplication, QMainWindow, QPushButton, QVBoxLayout, QWidget, QFileDialog, QMessageBox
from pyvistaqt import QtInteractor
import pyvista as pv
import numpy as np

# Импортируем скомпилированный нами C++ модуль бэкенда
try:
    import grdecl_core
except ImportError:
    print("[-] Error: grdecl_core module not found. Compile the C++ backend first!")
    sys.exit(1)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("GRDECL 3D Viewer [NOVATEK Project]")
        self.resize(1200, 800)

        # Главный виджет и компоновка
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)

        # Кнопка загрузки файла
        self.btn_load = QPushButton("Открыть .GRDECL файл", self)
        self.btn_load.clicked.connect(self.open_file_dialog)
        layout.addWidget(self.btn_load)

        # 3D-виджет от PyVista (встроенный в PyQt)
        self.plotter = QtInteractor(self)
        layout.addWidget(self.plotter.interactor)

        # Добавим приветственную 3D-сетку, чтобы сцена не была пустой
        self.show_welcome_box()

    def show_welcome_box(self):
        # Рисуем простой куб для инициализации сцены
        box = pv.Box(bounds=(0, 1, 0, 1, 0, 1))
        self.plotter.add_mesh(box, color="lightgray", show_edges=True)
        self.plotter.add_text("Load a .GRDECL file to render reservoir", font_size=12)
        self.plotter.reset_camera()

    def open_file_dialog(self):
        file_path, _ = QFileDialog.getOpenFileName(
            self, "Открыть модель пласта", "", "GRDECL Files (*.GRDECL *.grdecl);;All Files (*)"
        )
        
        if not file_path:
            return

        try:
            # 1. Создаем объект нашего C++ парсера
            parser = grdecl_core.Parser(file_path)
            
            # 2. Загружаем файл в память бэкенда
            if not parser.load():
                QMessageBox.critical(self, "Ошибка", f"Не удалось открыть файл:\n{file_path}")
                return

            # 3. Достаем SPECGRID (размеры NX, NY, NZ)
            dims = parser.get_specgrid()
            if len(dims) == 3:
                nx, ny, nz = dims
                print(f"[+] Loaded SPECGRID: NX={nx}, NY={ny}, NZ={nz}")
                QMessageBox.information(self, "Успех", f"Парсинг успешен!\nРазмеры сетки:\nNX: {nx}\nNY: {ny}\nNZ: {nz}")
                
                # Здесь в будущем мы вытащим ZCORN/COORD и построим реальный Grid пласта
                self.plotter.clear()
                self.plotter.add_text(f"Grid: {nx} x {ny} x {nz}", font_size=14)
                
                # Для теста нарисуем масштабированный бокс под размеры сетки
                grid_box = pv.Box(bounds=(0, nx*10, 0, ny*10, 0, nz*5))
                self.plotter.add_mesh(grid_box, color="orange", show_edges=True)
                self.plotter.reset_camera()
            else:
                QMessageBox.warning(self, "Внимание", "Ключевое слово SPECGRID не найдено или повреждено!")

        except Exception as e:
            QMessageBox.critical(self, "Критическая ошибка", f"Ошибка при обработке файла:\n{str(e)}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
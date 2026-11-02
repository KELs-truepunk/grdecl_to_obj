#эта часть кода была нагло написана ЛЛМ Гемнини 3.5, тк я знаю только класический Си
# даже плюсы не очень, а проект делать надо (может потом на ГТК4 сделаю, но пока на PyQt6)
#ядро на си++ писал почти сам, иногда подглядывая к нейронкам(я все равно не вайбкодер)
import sys
from PyQt6.QtWidgets import QApplication, QMainWindow, QPushButton, QVBoxLayout, QWidget, QFileDialog, QMessageBox
from pyvistaqt import QtInteractor
import pyvista as pv
import numpy as np

# Импортируем скомпилированный нами C++ модуль бэкенда
try:
    import grdecl_core
except ImportError:
    print("[-] Error: grdecl_to_obj module not found. Compile the C++ backend first!")
    sys.exit(1)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("GRDECL 3D Viewer")
        self.resize(1200, 800)

        # Сохраняем ссылки на данные сетки для экспорта
        self.current_dims = []
        self.current_coords = []
        self.current_zcorn = []

        # Главный виджет и компоновка
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)

        # Кнопка загрузки файла
        self.btn_load = QPushButton("Открыть .GRDECL файл", self)
        self.btn_load.clicked.connect(self.open_file_dialog)
        layout.addWidget(self.btn_load)

        # Кнопка экспорта в OBJ
        self.btn_export = QPushButton("Экспортировать в .OBJ", self)
        self.btn_export.clicked.connect(self.export_obj_file)
        layout.addWidget(self.btn_export)

        # 3D-виджет от PyVista (встроенный в PyQt)
        self.plotter = QtInteractor(self)
        layout.addWidget(self.plotter.interactor)

        # Добавим приветственную 3D-сетку, чтобы сцена не была пустой
        self.show_welcome_box()

    def show_welcome_box(self):
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
            parser = grdecl_core.Parser(file_path)
            if not parser.load():
                QMessageBox.critical(self, "Ошибка", f"Не удалось открыть файл:\n{file_path}")
                return

            dims = parser.get_specgrid()
            if len(dims) == 3:
                nx, ny, nz = dims
                print(f"[+] Loaded SPECGRID: NX={nx}, NY={ny}, NZ={nz}")

                # Достаем сырые массивы
                coords = parser.parse_keyword("COORD")
                zcorn = parser.parse_keyword("ZCORN")

                # Сохраняем в поля класса для последующего экспорта
                self.current_dims = dims
                self.current_coords = coords
                self.current_zcorn = zcorn

                # Считаем геометрию в C++ бэкенде
                raw_vertices = grdecl_core.GridBuilder.build_mesh_grid(dims, coords, zcorn)

                if not raw_vertices:
                    QMessageBox.warning(self, "Внимание", "Генератор сетки вернул пустой массив вершин!")
                    return

                points = np.array(raw_vertices).reshape(-1, 3)
                num_cells = len(points) // 8
                
                cells = np.hstack([
                    np.full((num_cells, 1), 8, dtype=np.int64),
                    np.arange(len(points), dtype=np.int64).reshape(num_cells, 8)
                ]).ravel()
                
                cell_types = np.full(num_cells, pv.CellType.HEXAHEDRON, dtype=np.uint8)
                grid = pv.UnstructuredGrid(cells, cell_types, points)

                self.plotter.clear()
                self.plotter.add_mesh(grid, show_edges=True, color="lightblue", scalars=None)
                self.plotter.add_text(f"Grid: {nx} x {ny} x {nz} ({num_cells} cells)", font_size=12)
                self.plotter.reset_camera()

                QMessageBox.information(self, "Успех", f"Построено ячеек: {num_cells}")
            else:
                QMessageBox.warning(self, "Внимание", "Ключевое слово SPECGRID не найдено или повреждено!")

        except Exception as e:
            QMessageBox.critical(self, "Критическая ошибка", f"Ошибка при обработке файла:\n{str(e)}")

    def export_obj_file(self):
        if not self.current_dims:
            QMessageBox.warning(self, "Предупреждение", "Сначала откройте валидный .grdecl файл!")
            return

        file_path, _ = QFileDialog.getSaveFileName(
            self, "Сохранить модель как OBJ", "", "Wavefront OBJ (*.obj);;All Files (*)"
        )
        if not file_path:
            return

        try:
            raw_vertices = grdecl_core.GridBuilder.build_mesh_grid(
                self.current_dims, self.current_coords, self.current_zcorn
            )
            success = grdecl_core.ObjExporter.export_to_obj(file_path, raw_vertices)
            
            if success:
                QMessageBox.information(self, "Успех", f"Модель успешно сохранена:\n{file_path}")
            else:
                QMessageBox.critical(self, "Ошибка", "Не удалось записать .obj файл.")
        except Exception as e:
            QMessageBox.critical(self, "Ошибка", f"Ошибка экспорта: {str(e)}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
# эта часть кода была нагло написана ЛЛМ Гемнини 3.5, тк я знаю только класический Си
# даже плюсы не очень, а проект делать надо (может потом на ГТК4 сделаю, но пока на PyQt6)
# ядро на си++ писал почти сам, иногда подглядывая к нейронкам(я все равно не вайбкодер)
import sys
from PyQt6.QtWidgets import (QApplication, QMainWindow, QPushButton, QVBoxLayout, 
                             QHBoxLayout, QWidget, QFileDialog, QMessageBox, 
                             QLabel, QDoubleSpinBox)
from pyvistaqt import QtInteractor
import pyvista as pv
import numpy as np

try:
    import grdecl_core
except ImportError:
    print("[-] Error: grdecl_core module not found. Compile the C++ backend first!")
    sys.exit(1)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("GRDECL 3D Viewer (MVP)")
        self.resize(1200, 800)

        self.current_dims = []
        self.current_coords = []
        self.current_zcorn = []
        self.current_actnum = [] # <-- Храним ACTNUM

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QHBoxLayout(central_widget)
        sidebar = QWidget()
        sidebar.setFixedWidth(280)
        sidebar_layout = QVBoxLayout(sidebar)

        self.btn_load = QPushButton("Открыть .GRDECL файл", sidebar)
        self.btn_load.clicked.connect(self.open_file_dialog)
        sidebar_layout.addWidget(self.btn_load)

        # --- Z-SCALE ---
        sidebar_layout.addWidget(QLabel("Масштаб по Z (Z-Scale):"))
        self.z_scale_spin = QDoubleSpinBox()
        self.z_scale_spin.setRange(1.0, 50.0)
        self.z_scale_spin.setSingleStep(1.0)
        self.z_scale_spin.setValue(1.0)
        self.z_scale_spin.valueChanged.connect(self.refresh_3d_view)
        sidebar_layout.addWidget(self.z_scale_spin)

        # --- ЭКСПОРТ ---
        sidebar_layout.addWidget(QLabel("--- Экспорт ---"))
        
        self.btn_export_obj = QPushButton("Экспортировать в .OBJ", sidebar)
        self.btn_export_obj.clicked.connect(lambda: self._export_format("OBJ"))
        sidebar_layout.addWidget(self.btn_export_obj)

        self.btn_export_stl = QPushButton("Экспортировать в .STL", sidebar)
        self.btn_export_stl.clicked.connect(lambda: self._export_format("STL"))
        sidebar_layout.addWidget(self.btn_export_stl)

        sidebar_layout.addStretch()
        layout.addWidget(sidebar)

        self.plotter = QtInteractor(self)
        layout.addWidget(self.plotter.interactor, 1)

        self.show_welcome_box()

    def show_welcome_box(self):
        box = pv.Box(bounds=(0, 1, 0, 1, 0, 1))
        self.plotter.add_mesh(box, color="lightgray", show_edges=True)
        self.plotter.add_axes()
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
                self.current_dims = dims
                self.current_coords = parser.parse_keyword("COORD")
                self.current_zcorn = parser.parse_keyword("ZCORN")
                self.current_actnum = parser.parse_keyword("ACTNUM") # <-- Читаем ACTNUM
                
                if not self.current_actnum:
                    print("[!] ACTNUM not found or empty. Drawing all cells.")

                self.refresh_3d_view()
            else:
                QMessageBox.warning(self, "Внимание", "SPECGRID не найден!")
        except Exception as e:
            QMessageBox.critical(self, "Ошибка", f"Ошибка при обработке файла:\n{str(e)}")

    def refresh_3d_view(self):
        if not self.current_dims:
            return

        try:
            z_val = self.z_scale_spin.value()
            
            # Вызываем C++ с новыми аргументами
            raw_vertices = grdecl_core.GridBuilder.build_mesh_grid(
                self.current_dims, 
                self.current_coords, 
                self.current_zcorn, 
                self.current_actnum, # <-- Передаем ACTNUM
                z_val                # <-- Передаем Z-Scale
            )

            if not raw_vertices:
                return

            points = np.array(raw_vertices).reshape(-1, 3)
            num_cells = len(points) // 8
            
            if num_cells == 0:
                self.plotter.clear()
                self.plotter.add_text("No active cells to display", font_size=12)
                return

            cells = np.hstack([
                np.full((num_cells, 1), 8, dtype=np.int64),
                np.arange(len(points), dtype=np.int64).reshape(num_cells, 8)
            ]).ravel()
            
            cell_types = np.full(num_cells, pv.CellType.HEXAHEDRON, dtype=np.uint8)
            grid = pv.UnstructuredGrid(cells, cell_types, points)

            self.plotter.clear()
            # Добавил show_edges=False, чтобы реальная сетка не превратилась в кашу из линий
            self.plotter.add_mesh(grid, show_edges=False, color="lightblue", scalars=None)
            self.plotter.add_axes()
            self.plotter.add_text(f"Grid: {self.current_dims[0]}x{self.current_dims[1]}x{self.current_dims[2]} | Cells: {num_cells} | Z-Scale: {z_val}x", font_size=10)
            self.plotter.reset_camera()

        except Exception as e:
            QMessageBox.critical(self, "Ошибка визуализации", str(e))

    def _export_format(self, fmt: str):
        if not self.current_dims:
            QMessageBox.warning(self, "Предупреждение", "Сначала откройте валидный .grdecl файл!")
            return

        filters = {
            "OBJ": "Wavefront OBJ (*.obj);;All Files (*)",
            "STL": "STL (*.stl);;All Files (*)"
        }
        file_path, _ = QFileDialog.getSaveFileName(self, f"Сохранить как {fmt}", "", filters[fmt])
        if not file_path:
            return

        try:
            z_val = self.z_scale_spin.value()
            
            raw_vertices = grdecl_core.GridBuilder.build_mesh_grid(
                self.current_dims, self.current_coords, self.current_zcorn, 
                self.current_actnum, z_val
            )
            
            if fmt == "OBJ":
                success = grdecl_core.ObjExporter.export_to_obj(file_path, raw_vertices)
            elif fmt == "STL":
                success = grdecl_core.StlExporter.export_to_stl(file_path, raw_vertices)
            else:
                success = False

            if success:
                QMessageBox.information(self, "Успех", f"Модель сохранена:\n{file_path}")
            else:
                QMessageBox.critical(self, "Ошибка", f"Не удалось записать файл.")
        except Exception as e:
            QMessageBox.critical(self, "Ошибка", f"Ошибка экспорта: {str(e)}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())
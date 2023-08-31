from os import scandir
from os.path import abspath, join, isdir, splitext
from tkinter import Tk, Toplevel
from tkinter.ttk import Treeview

from TkImgViewer import TkImgViewer


class TkTreeViewer:
    def __init__(self, dir: str = ".", parent=None):
        self.parent = parent
        self.dir = dir
        self.loadScreen()

    def loadScreen(self):
        try:
            self.window = Tk() if not self.parent else Toplevel(self.parent)
            self.window.title("TkTreeViewer")
            self.window.geometry("400x400")
            self.tree = Treeview(self.window)
            self.tree.heading("#0", text=self.dir)
            self.tree.bind("<<TreeviewOpen>>", self.__on_treeview_open)
            self.tree.bind("<<TreeviewSelect>>", self.__on_treeview_select)
            self.__load_tree("", abspath(self.dir))
            self.tree.pack(fill="both", expand=True)
            self.window.mainloop()
        except Exception as e:
            print("TkTreeViewer: [ERROR] loadScreen:", e)

    def __load_tree(self, parent: str, parent_path: str):
        is_empty = True
        for item in scandir(parent_path):
            is_empty = False
            item_path = join(parent_path, item.name)
            if item.is_dir():
                id = self.tree.insert(parent, "end", text=item.name, values=[item_path], open=False)
                self.__load_tree(id, item_path)
            else:
                self.tree.insert(parent, "end", text=item.name, values=[item_path])
        if is_empty:
            self.tree.insert(parent, "end", text="this is empty")

    def __on_treeview_open(self, event):
        selected = self.tree.selection()
        if selected:
            values = self.tree.item(selected, "values")
            if len(values):
                selected_path = values[0]
                if isdir(selected_path):
                    self.tree.delete(*self.tree.get_children(selected))
                    self.__load_tree(selected, selected_path)

    def __on_treeview_select(self, event):
        selected = self.tree.selection()
        if selected:
            values = self.tree.item(selected, "values")
            if len(values):
                selected_path = values[0]
                if not isdir(selected_path) and self.__is_image_file(selected_path):
                    TkImgViewer(selected_path, self.window)

    def __is_image_file(self, img_path: str):
        img_ext = [".jpeg", ".jpg", ".png", ".gif", ".bmp"]
        ext = splitext(img_path)[1].lower()
        return ext in img_ext

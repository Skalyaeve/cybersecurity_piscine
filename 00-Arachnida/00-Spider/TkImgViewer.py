from os.path import basename
from PIL.Image import open as PIL_open
from PIL.ImageTk import PhotoImage
from tkinter import Tk, Toplevel, Frame, Canvas, Label, Widget, Scrollbar
from PIL.ImageSequence import Iterator as PIL_iterator
import pyexiv2


class TkImgViewer:
    def __init__(self, img_path: str, parent=None):
        self.parent = parent
        self.load_img_data(img_path)
        self.load_screen()

    def load_screen(self):
        if not self.img:
            print("TkImgViewer: [ERROR] load_screen(): please load an image first")
            return
        self.window = Tk() if not self.parent else Toplevel(self.parent)
        screen_width = self.window.winfo_screenwidth()
        screen_height = self.window.winfo_screenheight()
        self.img_width = self.img.width if self.img.width < screen_width - 400 else screen_width - 400
        self.img_height = self.img.height if self.img.height < screen_height - 150 else screen_height - 150

        self.window.title("TkImgViewer")
        self.__load_infos()
        self.__load_img()
        self.window.mainloop()

    def load_img_data(self, img_path: str):
        try:
            self.img_path = img_path
            metadata = pyexiv2.Image(self.img_path)
            try:
                try:
                    self.exif = []
                    exif_data = metadata.read_exif()
                    for key, value in exif_data.items():
                        self.exif.append((key, value))
                except Exception as e:
                    pass

                try:
                    self.iptc = []
                    iptc_data = metadata.read_iptc()
                    for key, value in iptc_data.items():
                        self.iptc.append((key, value))
                except Exception as e:
                    pass

                try:
                    self.xmp = []
                    xmp_data = metadata.read_xmp()
                    for key, value in xmp_data.items():
                        self.xmp.append((key, value))
                except Exception as e:
                    pass

                metadata.close()
                self.img = PIL_open(self.img_path)
            except Exception as e:
                print("TkImgViewer: load_img_data(): can't get metadata: ", e)
        except Exception as e:
            print(f"TkImgViewer: [ERROR] load_img_data(): ", e)

    def __load_infos(self):
        frame_width = 300
        left_frame = Frame(self.window, width=frame_width, height=self.img_height)
        left_frame.pack(side="left", fill="y")

        x_scrollbar = Scrollbar(left_frame, orient="horizontal")
        x_scrollbar.pack(side="bottom", fill="x")
        y_scrollbar = Scrollbar(left_frame, orient="vertical")
        y_scrollbar.pack(side="right", fill="y")

        info_canvas = Canvas(left_frame, width=frame_width, height=self.img_height)
        info_canvas.config(xscrollcommand=x_scrollbar.set, yscrollcommand=y_scrollbar.set)
        info_canvas.pack(side="left", fill="y")

        x_scrollbar.config(command=info_canvas.xview)
        y_scrollbar.config(command=info_canvas.yview)

        content_frame = Frame(info_canvas)
        info_canvas.create_window((0, 0), window=content_frame, anchor="nw")

        filename_label = Label(content_frame, text=f"{basename(self.img_path)}:", justify="left")
        filename_label.pack(anchor="w")

        size_label = Label(content_frame, text=f"Size: {self.img.size}", justify="left")
        size_label.pack(anchor="w")

        if self.exif:
            for x, y in self.exif:
                data_label = Label(content_frame, text=f"{x}: {y}", justify="left")
                data_label.pack(anchor="w")
        if self.iptc:
            for x, y in self.iptc:
                data_label = Label(content_frame, text=f"{x}: {y}", justify="left")
                data_label.pack(anchor="w")
        if self.xmp:
            for x, y in self.xmp:
                data_label = Label(content_frame, text=f"{x}: {y}", justify="left")
                data_label.pack(anchor="w")

        content_frame.update_idletasks()
        info_canvas.config(scrollregion=info_canvas.bbox("all"))
        if self.exif or self.iptc or self.xmp:
            data_label.pack(anchor="w")

    def __load_img(self):
        frame = Frame(self.window)
        frame.pack(side="left")

        x_scroll = Scrollbar(frame, orient="horizontal")
        x_scroll.pack(side="bottom", fill="x")
        y_scroll = Scrollbar(frame, orient="vertical")
        y_scroll.pack(side="right", fill="y")

        img_canvas = Canvas(frame, width=self.img_width, height=self.img_height, scrollregion=(0, 0, self.img.width, self.img.height))
        img_canvas.pack(side="left")

        img_canvas.config(xscrollcommand=x_scroll.set, yscrollcommand=y_scroll.set)
        x_scroll.config(command=img_canvas.xview)
        y_scroll.config(command=img_canvas.yview)

        if self.img.format != "GIF":
            self.displayed_img = PhotoImage(self.img)
            img_canvas.create_image(0, 0, anchor="nw", image=self.displayed_img)
        else:
            self.__display_animated_img(img_canvas)

    def __display_animated_img(self, canvas: Widget):
        frames = []
        for frame in PIL_iterator(self.img):
            frames.append(PhotoImage(frame))
        self.__play_animation(canvas, frames)

    def __play_animation(self, canvas: Widget, frames: list):
        num_frames = len(frames)
        delay = self.img.info.get("duration", 50)

        def update_frame(index):
            if index < num_frames:
                canvas.create_image(0, 0, anchor="nw", image=frames[index])
                self.window.after(delay, update_frame, (index + 1) % num_frames)
            else:
                canvas.delete("all")
                self.__display_animated_img(canvas)

        update_frame(0)

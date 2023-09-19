from os.path import basename
from PIL.Image import open as PIL_open
from PIL.ImageTk import PhotoImage
from PIL.ExifTags import TAGS
from piexif import load
from tkinter import Tk, Toplevel, Frame, Canvas, Label, Widget
from PIL.ImageSequence import Iterator as PIL_iterator


class TkImgViewer:
    def __init__(self, img_path: str, parent=None):
        self.parent = parent
        self.loadImgData(img_path)
        self.loadScreen()

    def loadScreen(self):
        try:
            self.window = Tk() if not self.parent else Toplevel(self.parent)
            self.window.title("TkImgViewer")
            self.__load_infos()
            self.__load_img()
            self.window.mainloop()
        except Exception as e:
            print(f"TkImgViewer: [ERROR] loadScreen():", e)

    def loadImgData(self, img_path: str):
        try:
            self.img_path = img_path
            self.img = PIL_open(self.img_path)
            try:
                self.exif = load(self.img_path)
            except Exception as e:
                self.exif = None
                print("TkImgViewer: loadImgData(): can't get EXIF data:", e)
        except Exception as e:
            print(f"TkImgViewer: [ERROR] loadImgData():", e)

    def __load_infos(self):
        left_frame = Frame(self.window)
        left_frame.pack(side="left", fill="y", expand=True)
        canvas = Canvas(self.window, width=self.img.width, height=self.img.height)
        canvas.pack(side="left")
        metadata_label = Label(left_frame, text=f"{basename(self.img_path)}:", justify="left")
        metadata_label.pack(anchor="w")
        size_label = Label(left_frame, text=f"Size: {self.img.size}", justify="left")
        size_label.pack(anchor="w")
        if self.exif and self.exif.len():
            for tag in self.exif:
                if self.exif[tag]:
                    for x in self.exif[tag]:
                        value = self.exif[tag][x].decode("utf-8")
                        exif_label = Label(left_frame, text=f"{TAGS[x]}: {value}", justify="left")
                        exif_label.pack(anchor="w")
        else:
            no_exif_label = Label(left_frame, text="No EXIF data", justify="left")
            no_exif_label.pack(anchor="w")

    def __load_img(self):
        img_width, img_height = self.img.size
        canvas = self.window.winfo_children()[1]
        canvas.config(width=img_width, height=img_height)
        if self.img.format != "GIF":
            self.__display_static_img(canvas)
        else:
            self.__display_animated_img(canvas)

    def __display_static_img(self, canvas: Widget):
        img_width, img_height = self.img.size
        canvas.config(width=img_width, height=img_height)
        self.displayed_img = PhotoImage(self.img)
        canvas.create_image(0, 0, anchor="nw", image=self.displayed_img)

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

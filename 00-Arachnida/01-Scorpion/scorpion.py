from os.path import basename as os_path_basename
from PIL import Image as PIL_Image
from PIL.ImageSequence import Iterator as PIL_ImageSequence_Iterator
from PIL.ImageTk import PhotoImage as PIL_ImageTk_PhotoImage
from tkinter import Tk as tkinter_Tk
from tkinter import Toplevel as tkinter_Toplevel
from tkinter import Frame as tkinter_Frame
from tkinter import Canvas as tkinter_Canvas
from tkinter import Label as tkinter_Label
from tkinter import Scrollbar as tkinter_Scrollbar
from pyexiv2 import Image as pyexiv2_Image


class Scorpion:
    def __init__(self, img_path: str):
        self.load_data(img_path)

    def load_data(self, img_path: str):
        if not img_path:
            print(f"Scorpion: [ ERROR ] load_data({img_path}):\nNo image path provided.")
            return

        self.path = img_path
        self.width = None
        self.height = None
        self.format = None
        self.exif = {}
        self.iptc = {}
        self.xmp = {}

        try:
            img = PIL_Image.open(self.path)
            self.width = img.width
            self.height = img.height
            self.format = img.format
            img.close()
        except Exception as e:
            print(f"Scorpion: [ ERROR ] load_data({img_path}):\nPIL_Image_open({self.path}): {e}")

        try:
            metadata = pyexiv2_Image(self.path)
            try:
                self.exif = metadata.read_exif()
            except Exception as e:
                print(f"Scorpion: [ ERROR ] load_data({img_path}):\nmetadata.read_exif(): {e}")

            try:
                self.iptc = metadata.read_iptc()
            except Exception as e:
                print(f"Scorpion: [ ERROR ] load_data({img_path}):\nmetadata.read_iptc(): {e}")

            try:
                self.xmp = metadata.read_xmp()
            except Exception as e:
                print(f"Scorpion: [ ERROR ] load_data({img_path}):\nmetadata.read_xmp(): {e}")

            metadata.close()
        except Exception as e:
            print(f"Scorpion: [ ERROR ] load_data({img_path}):\npyexiv2_Image({self.path}): {e}")

    def show_metadata(self, graphic: bool = False):
        if not self.path:
            print(f"Scorpion: [ ERROR ] show_metadata():\nPlease load_data() first.")
            return

        if graphic:
            return self.__load_gui()

        print("------------------ S C O R P I O N ------------------")
        print(f"Filename: {os_path_basename(self.path)}")
        print(f"Width: {self.width}")
        print(f"Height: {self.height}")

        if len(self.exif):
            print("--------- E X I F ---------")
            for x, y in self.exif.items():
                print(f"{x}: {y}")

        if len(self.iptc):
            print("--------- I P T C ---------")
            for x, y in self.iptc.items():
                print(f"{x}: {y}")

        if len(self.xmp):
            print("---------- X M P ----------")
            for x, y in self.xmp.items():
                print(f"{x}: {y}")
        print("-----------------------------------------------------")

    def modify_metadata(self, key: str, value: str):
        if not self.path:
            print(f"Scorpion: [ ERROR ] modify_metadata({key}, {value}):\nPlease load_data() first.")
            return
        if not key:
            print(f"Scorpion: [ ERROR ] modify_metadata({key}, {value}):\nNo key provided.")
            return

        try:
            metadata = pyexiv2_Image(self.path)
            metadata_type = key.split(".")[0]

            if metadata_type == "Exif":
                self.exif[key] = value
                try:
                    metadata.modify_exif(self.exif)
                except Exception as e:
                    print(f"Scorpion: [ ERROR ] modify_metadata({key}, {value}):\nmetadata.modify_exif(): {e}")

            elif metadata_type == "Iptc":
                self.iptc[key] = value
                try:
                    metadata.modify_iptc(self.iptc)
                except Exception as e:
                    print(f"Scorpion: [ ERROR ] modify_metadata({key}, {value}):\nmetadata.modify_iptc(): {e}")

            elif metadata_type == "Xmp":
                self.xmp[key] = value
                try:
                    metadata.modify_xmp(self.xmp)
                except Exception as e:
                    print(f"Scorpion: [ ERROR ] modify_metadata({key}, {value}):\nmetadata.modify_xmp(): {e}")

            metadata.close()
        except Exception as e:
            print(f"Scorpion: [ ERROR ] modify_metadata({key}, {value}):\npyexiv2_Image({self.path}): {e}")

    def delete_metadata(self, key: str):
        if not self.path:
            print(f"Scorpion: [ ERROR ] delete_metadata({key}):\nPlease load_data() first.")
            return
        if not key:
            raise (f"Scorpion: [ ERROR ] delete_metadata({key}):\nNo key provided.")

        if self.has_metadata(key):
            try:
                metadata = pyexiv2_Image(self.path)
                metadata_type = key.split(".")[0]

                if metadata_type == "Exif":
                    del self.exif[key]
                    try:
                        metadata.modify_exif(self.exif)
                    except Exception as e:
                        print(f"Scorpion: [ ERROR ] delete_metadata({key}):\nmetadata.modify_exif(): {e}")

                elif metadata_type == "Iptc":
                    del self.iptc[key]
                    try:
                        metadata.modify_iptc(self.iptc)
                    except Exception as e:
                        print(f"Scorpion: [ ERROR ] delete_metadata({key}):\nmetadata.modify_iptc(): {e}")

                elif metadata_type == "Xmp":
                    del self.xmp[key]
                    try:
                        metadata.modify_xmp(self.xmp)
                    except Exception as e:
                        print(f"Scorpion: [ ERROR ] delete_metadata({key}):\nmetadata.modify_xmp(): {e}")

                metadata.close()
            except Exception as e:
                print(f"Scorpion: [ ERROR ] delete_metadata({key}):\npyexiv2_Image({self.path}): {e}")
        else:
            print(f"Scorpion: [ ERROR ] delete_metadata({key}):\n{key} not found.")

    def has_metadata(self, key: str):
        if not self.path:
            print(f"Scorpion: [ ERROR ] delete_metadata({key}):\nPlease load_data() first.")
            return
        if not key:
            print(f"Scorpion: [ ERROR ] delete_metadata({key}):\nNo key provided.")
            return

        for metadata in [self.exif, self.iptc, self.xmp]:
            if key in metadata:
                return True
        return False

    def __load_gui(self, parent=None):
        self.gui = {
            "parent": parent,
            "window": None,
            "screen_width": None,
            "screen_height": None,
            "img": None,
        }
        try:
            self.gui["window"] = tkinter_Tk() if not self.gui["parent"] else tkinter_Toplevel(self.gui["parent"])
        except Exception as e:
            print(f"Scorpion: [ ERROR ] __load_gui(): Can't initialise tkinter: {e}")
            return

        self.gui["screen_width"] = self.gui["window"].winfo_screenwidth()
        self.gui["screen_height"] = self.gui["window"].winfo_screenheight()

        try:
            self.__load_infos()
        except Exception as e:
            print(f"Scorpion: [ ERROR ] __load_gui():\nCan't load image infos: {e}")
            return

        try:
            self.__load_img()
        except Exception as e:
            print(f"Scorpion: [ ERROR ] __load_gui():\nCan't load image: {e}")
            return

        self.gui["window"].title("S C O R P I O N")
        self.gui["window"].mainloop()

    def __load_infos(self):
        frame_width = 400
        frame_height = self.height if self.height < self.gui["screen_height"] - 150 else self.gui["screen_height"] - 150

        base_frame = tkinter_Frame(self.gui["window"])
        base_frame.pack(side="left")

        x_scrollbar = tkinter_Scrollbar(base_frame, orient="horizontal")
        x_scrollbar.pack(side="bottom", fill="x")
        y_scrollbar = tkinter_Scrollbar(base_frame, orient="vertical")
        y_scrollbar.pack(side="right", fill="y")

        base_canvas = tkinter_Canvas(base_frame, width=frame_width, height=frame_height)
        base_canvas.config(xscrollcommand=x_scrollbar.set, yscrollcommand=y_scrollbar.set)
        base_canvas.pack(side="left", fill="y")

        x_scrollbar.config(command=base_canvas.xview)
        y_scrollbar.config(command=base_canvas.yview)

        content_frame = tkinter_Frame(base_canvas)
        base_canvas.create_window((0, 0), window=content_frame, anchor="nw")

        filename_label = tkinter_Label(content_frame, text=f"Filename: {os_path_basename(self.path)}", justify="left")
        filename_label.pack(anchor="w")
        width_label = tkinter_Label(content_frame, text=f"Width: {self.width}", justify="left")
        width_label.pack(anchor="w")
        height_label = tkinter_Label(content_frame, text=f"Height: {self.height}", justify="left")
        height_label.pack(anchor="w")

        if len(self.exif):
            exif_label = tkinter_Label(content_frame, text=f"--------- E X I F ---------", justify="left")
            exif_label.pack(anchor="w")
            for x, y in self.exif.items():
                data_label = tkinter_Label(content_frame, text=f"{x}: {y}", justify="left")
                data_label.pack(anchor="w")

        if len(self.iptc):
            iptc_label = tkinter_Label(content_frame, text=f"--------- I P T C ---------", justify="left")
            iptc_label.pack(anchor="w")
            for x, y in self.iptc.items():
                data_label = tkinter_Label(content_frame, text=f"{x}: {y}", justify="left")
                data_label.pack(anchor="w")

        if len(self.xmp):
            xmp_label = tkinter_Label(content_frame, text=f"---------- X M P ----------", justify="left")
            xmp_label.pack(anchor="w")
            for x, y in self.xmp.items():
                data_label = tkinter_Label(content_frame, text=f"{x}: {y}", justify="left")
                data_label.pack(anchor="w")

        content_frame.update_idletasks()
        base_canvas.config(scrollregion=base_canvas.bbox("all"))

    def __load_img(self):
        frame_width = self.width if self.width < self.gui["screen_width"] - 500 else self.gui["screen_width"] - 500
        frame_height = self.height if self.height < self.gui["screen_height"] - 150 else self.gui["screen_height"] - 150

        base_frame = tkinter_Frame(self.gui["window"])
        base_frame.pack(side="left")

        x_scroll = tkinter_Scrollbar(base_frame, orient="horizontal")
        x_scroll.pack(side="bottom", fill="x")
        y_scroll = tkinter_Scrollbar(base_frame, orient="vertical")
        y_scroll.pack(side="right", fill="y")

        base_canvas = tkinter_Canvas(base_frame, width=frame_width, height=frame_height, scrollregion=(0, 0, self.width, self.height))
        base_canvas.config(xscrollcommand=x_scroll.set, yscrollcommand=y_scroll.set)
        base_canvas.pack(side="left")

        x_scroll.config(command=base_canvas.xview)
        y_scroll.config(command=base_canvas.yview)

        img = PIL_Image.open(self.path)
        if self.format == "GIF":
            frames = [PIL_ImageTk_PhotoImage(frame) for frame in PIL_ImageSequence_Iterator(img)]
            delay = img.info.get("duration", 50)
            img.close()
            self.__update_frame(base_canvas, frames, len(frames), delay, 0)
        else:
            self.gui["img"] = PIL_ImageTk_PhotoImage(img)
            img.close()
            base_canvas.create_image(0, 0, anchor="nw", image=self.gui["img"])

    def __update_frame(self, canvas: tkinter_Canvas, frames: list, count: int, delay: int, index: int):
        if self.gui["img"]:
            canvas.delete(self.gui["img"])
        self.gui["img"] = canvas.create_image(0, 0, anchor="nw", image=frames[index])

        self.gui["window"].after(delay, self.__update_frame, canvas, frames, count, delay, (index + 1) % count)

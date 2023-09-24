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
from tkinter import Entry as tkinter_Entry
from tkinter import Button as tkinter_Button
from tkinter import Text as tkinter_Text
from tkinter import DISABLED as tkinter_DISABLED
from pyexiv2 import Image as pyexiv2_Image
from pyexiv2 import metadata as pyexiv2_metadata


class Scorpion:
    def __init__(self, img_path: str):
        self.load_data(img_path)

    def load_data(self, img_path: str):
        self.path = None
        self.width = None
        self.height = None
        self.format = None
        self.exif = {}
        self.iptc = {}
        self.xmp = {}

        try:
            img = PIL_Image.open(img_path)
            self.path = img_path
            self.width = img.width
            self.height = img.height
            self.format = img.format
            img.close()

            self.__load_metadata()
        except Exception as e:
            print(f"Scorpion: [ ERROR ] load_data({img_path}): PIL_Image.open({self.path}): {e}")

    def __load_metadata(self):
        try:
            metadata = pyexiv2_Image(self.path)
            try:
                self.exif = metadata.read_exif()
            except Exception as e:
                print(f"Scorpion: [ ERROR ] __load_metadata({self.path}): metadata.read_exif(): {e}")

            try:
                self.iptc = metadata.read_iptc()
            except Exception as e:
                print(f"Scorpion: [ ERROR ] __load_metadata({self.path}): metadata.read_iptc(): {e}")

            try:
                self.xmp = metadata.read_xmp()
            except Exception as e:
                print(f"Scorpion: [ ERROR ] __load_metadata({self.path}): metadata.read_xmp(): {e}")

            metadata.close()
        except Exception as e:
            print(f"Scorpion: [ ERROR ] __load_metadata({self.path}): pyexiv2_Image({self.path}): {e}")

    def show_metadata(self, graphic: bool = False):
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

    def modify_metadata(self, key: str, value: str | None):
        try:
            if not self.__update_metadata(key, value):
                return
            self.__load_metadata()
        except Exception as e:
            print(f"Scorpion: [ ERROR ] modify_metadata({key}, {value}): pyexiv2_Image({self.path}): {e}")

    def __update_metadata(self, key: str, value: str):
        metadata = pyexiv2_metadata.ImageMetadata(self.path)
        metadata.read()
        try:
            if not value:
                del metadata[key]
                metadata.write()
                return True

            try:
                metadata[key] = str(value)
                metadata.write()
                return True
            except:
                pass

            try:
                metadata[key] = [str(value)]
                metadata.write()
                return True
            except:
                pass

            try:
                metadata[key] = int(value)
                metadata.write()
                return True
            except:
                pass

            try:
                metadata[key] = [int(value)]
                metadata.write()
                return True
            except:
                pass

            try:
                metadata[key] = float(value)
                metadata.write()
                return True
            except:
                pass

            try:
                metadata[key] = [float(value)]
                metadata.write()
                return True
            except:
                pass

            try:
                metadata[key] = self.__str_to_bool(value)
                metadata.write()
                return True
            except:
                pass

            try:
                metadata[key] = [self.__str_to_bool(value)]
                metadata.write()
                return True
            except:
                pass

            try:
                metadata[key] = value
                metadata.write()
                return True
            except Exception as e:
                raise e
        except Exception as e:
            print(f"Scorpion: [ ERROR ] __update_metadata({key}, {value}): Can't update metadata: {e}")
            return False

    def __str_to_bool(self, value: str):
        value = value.lower()
        if value not in ["true", "false"]:
            raise ValueError("Not a boolean")
        return value == "true"

    def data_exists(self, key: str):
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
            "img_infos": {},
            "new_key": None,
            "new_value": None,
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
            frame_width = 400
            frame_height = self.height if self.height < self.gui["screen_height"] - 150 else self.gui["screen_height"] - 150
            infos_frame = tkinter_Frame(self.gui["window"], name="infos", width=frame_width, height=frame_height)
            infos_frame.pack(side="left", fill="both", expand=True)
            infos_frame.pack_propagate(False)
            self.__load_infos(infos_frame)
        except Exception as e:
            print(f"Scorpion: [ ERROR ] __load_gui(): Can't load image infos: {e}")
            return

        try:
            self.__load_img()
        except Exception as e:
            print(f"Scorpion: [ ERROR ] __load_gui(): Can't load image: {e}")
            return

        self.gui["window"].title("S C O R P I O N")
        self.gui["window"].mainloop()

    def __load_infos(self, parent: tkinter_Frame):
        base_frame = tkinter_Frame(parent)
        base_frame.pack(side="left", fill="both", expand=True)

        x_scrollbar = tkinter_Scrollbar(base_frame, orient="horizontal")
        x_scrollbar.pack(side="bottom", fill="x")
        y_scrollbar = tkinter_Scrollbar(base_frame, orient="vertical")
        y_scrollbar.pack(side="right", fill="y")

        base_canvas = tkinter_Canvas(base_frame)
        base_canvas.config(xscrollcommand=x_scrollbar.set, yscrollcommand=y_scrollbar.set)
        base_canvas.pack(side="left", fill="both", expand=True)

        x_scrollbar.config(command=base_canvas.xview)
        y_scrollbar.config(command=base_canvas.yview)

        content_frame = tkinter_Frame(base_canvas)
        base_canvas.create_window((0, 0), window=content_frame, anchor="nw")

        filename_label = tkinter_Label(content_frame, text=f"Filename: {os_path_basename(self.path)}")
        filename_label.pack(anchor="w")
        width_label = tkinter_Label(content_frame, text=f"Width: {self.width}")
        width_label.pack(anchor="w")
        height_label = tkinter_Label(content_frame, text=f"Height: {self.height}")
        height_label.pack(anchor="w")

        if len(self.exif):
            exif_label = tkinter_Label(content_frame, text=f"[ E X I F ]")
            exif_label.pack(anchor="w", pady=(5, 0))
            for x, y in self.exif.items():
                self.__pack_info(content_frame, x, y)

        if len(self.iptc):
            iptc_label = tkinter_Label(content_frame, text=f"[ I P T C ]")
            iptc_label.pack(anchor="w", pady=(5, 0))
            for x, y in self.iptc.items():
                self.__pack_info(content_frame, x, y)

        if len(self.xmp):
            xmp_label = tkinter_Label(content_frame, text=f"[ X M P ]")
            xmp_label.pack(anchor="w", pady=(5, 0))
            for x, y in self.xmp.items():
                self.__pack_info(content_frame, x, y)

        modify_button = tkinter_Button(content_frame, text="S A V E", command=self.__update_list, width=10)
        modify_button.pack(padx=(145, 0), pady=(5, 0), anchor="w")

        data_label = tkinter_Label(content_frame, text=f"-------------------------------------------------------------------------------")
        data_label.pack(anchor="w", pady=5)

        data_text = tkinter_Text(content_frame, height=1, width=len("https://exiv2.org/metadata.html"))
        data_text.insert("1.0", "https://exiv2.org/metadata.html")
        data_text.config(state=tkinter_DISABLED)
        data_text.pack(padx=(85, 0), anchor="w")

        entry_frame = tkinter_Frame(content_frame)
        entry_frame.pack(padx=(18, 0), anchor="w")
        key_label = tkinter_Label(entry_frame, text=f"KEY ")
        key_label.pack(side="left")
        self.gui["new_key"] = tkinter_Entry(entry_frame, width=35)
        self.gui["new_key"].pack(side="left")

        entry_frame = tkinter_Frame(content_frame)
        entry_frame.pack(anchor="w")
        value_label = tkinter_Label(entry_frame, text=f"VALUE ")
        value_label.pack(side="left")
        self.gui["new_value"] = tkinter_Entry(entry_frame, width=35)
        self.gui["new_value"].pack(side="left")

        modify_button = tkinter_Button(content_frame, text="A D D", command=self.__add_info, width=10)
        modify_button.pack(padx=(145, 0), pady=5, anchor="w")

        content_frame.update_idletasks()
        base_canvas.config(scrollregion=base_canvas.bbox("all"))

    def __pack_info(self, content_frame: tkinter_Frame, x: str, y: str):
        data_frame = tkinter_Frame(content_frame)
        data_frame.pack(anchor="w")
        data_label = tkinter_Label(data_frame, text=f"{x} ")
        data_label.pack(side="left")
        data_entry = tkinter_Entry(data_frame, width=50)
        data_entry.insert(0, y)
        data_entry.pack(anchor="w")
        self.gui["img_infos"][x] = data_entry

    def __add_info(self):
        key = self.gui["new_key"].get()
        value = self.gui["new_value"].get()
        if not key or not value:
            print(f"Scorpion: [ INFO ] __add_info({key}, {value}): Empty key or value.")
            return
        if self.data_exists(key):
            print(f"Scorpion: [ ERROR ] __add_info({key}, {value}): Key already exists.")
            return
        self.modify_metadata(key, value)
        self.__refresh_infos()

    def __update_list(self):
        for key, value in self.gui["img_infos"].items():
            value = value.get()
            if not value:
                self.modify_metadata(key, None)
            elif key in self.exif and value != str(self.exif[key]):
                self.modify_metadata(key, value)
            elif key in self.iptc and value != str(self.iptc[key]):
                self.modify_metadata(key, value)
            elif key in self.xmp and value != str(self.xmp[key]):
                self.modify_metadata(key, value)
        self.gui["img_infos"].clear()
        self.__refresh_infos()

    def __refresh_infos(self):
        for widget in self.gui["window"].winfo_children()[0].winfo_children():
            widget.destroy()
        self.__load_infos(self.gui["window"].winfo_children()[0])

    def __load_img(self):
        frame_width = self.width if self.width < self.gui["screen_width"] - 500 else self.gui["screen_width"] - 500
        frame_height = self.height if self.height < self.gui["screen_height"] - 150 else self.gui["screen_height"] - 150

        base_frame = tkinter_Frame(self.gui["window"], name="image", width=frame_width, height=frame_height)
        base_frame.pack(side="left", fill="both", expand=True)

        x_scroll = tkinter_Scrollbar(base_frame, orient="horizontal")
        x_scroll.pack(side="bottom", fill="x")
        y_scroll = tkinter_Scrollbar(base_frame, orient="vertical")
        y_scroll.pack(side="right", fill="y")

        base_canvas = tkinter_Canvas(base_frame, width=frame_width, height=frame_height, scrollregion=(0, 0, self.width, self.height))
        base_canvas.config(xscrollcommand=x_scroll.set, yscrollcommand=y_scroll.set)
        base_canvas.pack(side="left", fill="both", expand=True)

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

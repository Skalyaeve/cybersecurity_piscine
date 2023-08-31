from PIL import Image
from PIL.ExifTags import TAGS
from piexif import load, dump, insert

from TkImgViewer import TkImgViewer


class ExifManager:
    def __init__(self, img_path: str):
        self.load(img_path)

    def load(self, img_path: str):
        try:
            self.img_path = img_path
            self.img = Image.open(self.img_path)
            self.exif = load(self.img_path)
        except Exception as e:
            print("ExifManager: [ERROR] __init__:", e)
            self.img = None
            self.exif = None

    def show(self, graphic: bool = False):
        try:
            if graphic:
                TkImgViewer(self.img_path)
                return
            if not self.img:
                print("ExifManager: invalid image, try to instance.load(anotherImage)")
                return
            print("---")
            print(f"Metadata for {self.img_path}:")
            print(f"Size: {self.img.size}")
            if not self.exif:
                print("no EXIF data")
                return
            for tag in self.exif:
                if self.exif[tag]:
                    for x in self.exif[tag]:
                        value = self.exif[tag][x].decode("utf-8")
                        print(f"{TAGS[x]}:", value)
            print("---")
        except Exception as e:
            print(f"TkImgViewer: [ERROR] show():", e)

    def modify(self, key: str, value: str):
        if not self.img:
            print("ExifManager: invalid image, try to instance.load(anotherImage)")
            return
        try:
            id = [x for x, y in TAGS.items() if y == key][0]
            for tag in self.exif:
                if self.exif[tag] is not None:
                    self.exif[tag][id] = value
                    try:
                        bytes = dump(self.exif)
                        insert(bytes, self.img_path)
                        break
                    except:
                        continue
            else:
                print(f"ExifManager({self.img_path}): can't modify: invalid value ({key}: {value})")
            self.exif = load(self.img_path)
        except:
            print(f"ExifManager: [ERROR] modify() failed: invalid EXIF data")

    def delete(self, key: str):
        if not self.img:
            print("ExifManager: invalid image, try to instance.load(anotherImage)")
            return
        try:
            founded = False
            for tag in self.exif:
                if self.exif[tag]:
                    for x in self.exif[tag]:
                        if TAGS[x] == key:
                            founded = True
                            del self.exif[tag][x]
                            break
                    if founded:
                        break
            else:
                print(f"ExifManager({self.img_path}): can't delete: invalid value ({key})")
            bytes = dump(self.exif)
            insert(bytes, self.img_path)
            self.exif = load(self.img_path)
        except Exception as e:
            print(f"TkImgViewer: [ERROR] show():", e)

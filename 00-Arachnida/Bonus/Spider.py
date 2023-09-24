from os import makedirs as os_makedir
from os import scandir as os_scandir
from os.path import basename as os_path_basename
from os.path import abspath as os_path_abspath
from os.path import join as os_path_join
from os.path import isdir as os_path_isdir
from os.path import splitext as os_path_splitext
from os.path import exists as os_path_exists
from requests import get as requests_get
from bs4 import BeautifulSoup as bs4_BeautifulSoup
from urllib.parse import urlparse as urllib_parse_urlparse
from tkinter import Tk as tkinter_Tk
from tkinter import Toplevel as tkinter_Toplevel
from tkinter.ttk import Treeview as ttk_Treeview

from Scorpion import Scorpion

c_green = "\033[92m"
c_red = "\033[91m"
c_stop = "\033[0m"


class Node:
    def __init__(self, base_url: str, url: str, parent=None):
        if url[-1] == "/":
            url = url[:-1]
        self.base_url = base_url
        self.url = url
        self.html = None
        self.parsed = False
        self.parent = parent
        self.childs = []

    def create_html(self):
        try:
            response = requests_get("https://" + self.url)
            try:
                self.html = bs4_BeautifulSoup(response.text, "html.parser")
                return True
            except Exception as e:
                print(f'Spider: [ ERROR ] BeautifulSoup({response.text}, "html.parser"): {e}')
                return False
        except Exception as e:
            print(f"Spider: [ ERROR ] requests_get({self.url}): {e}")
            return False

    def create_childs(self):
        try:
            print(f"Spider: [ INFO ] Creating child nodes of {self.url}...")
            links = self.html.find_all("a")
            for link in links:
                url: str = link.get("href")

                if (not url) or (url[0] == "#"):
                    continue
                if url[0] == "/":
                    url = self.base_url + url
                if url[-1] == "/":
                    url = url[:-1]

                url = url.lower()
                url = urllib_parse_urlparse(url)
                netloc = url.netloc.replace("www.", "")
                netloc = url.netloc.split(":")[0]
                if netloc != self.base_url:
                    continue

                url = netloc + url.path
                if url == self.url:
                    continue
                if any(child.url == url for child in self.childs):
                    continue

                root_node = self.parent
                while root_node and root_node.parent:
                    root_node = root_node.parent
                if self.__link_already_exist(url, root_node):
                    continue

                print(url)
                self.childs.append(Node(self.base_url, url, self))

            if len(self.childs):
                print(f"Spider: [ INFO ] Child nodes of {self.url} {c_green}created{c_stop}. ({len(self.childs)})\n")
            else:
                print(f"Spider: [ INFO ] {self.url} have {c_green}no childs{c_stop}.\n")
        except Exception as e:
            print(f"Spider: [ ERROR ] create_childs(): {e}")

    def __link_already_exist(self, url: str, node):
        if not node:
            return False
        if url == node.url:
            return True
        for child in node.childs:
            if node.__link_already_exist(url, child):
                return True
        return False


class Spider:
    def __init__(self, url: str, img_dir: str = "./"):
        self.img_ext = [".jpeg", ".jpg", ".png", ".gif", ".bmp"]
        self.img_dir = img_dir
        self.root_node = None
        self.node = None
        try:
            url = url.lower()
            url = urllib_parse_urlparse(url)
            netloc = url.netloc.replace("www.", "")
            netloc = url.netloc.split(":")[0]
            self.root_node = Node(netloc, netloc + url.path)
            self.node = self.root_node
        except Exception as e:
            print(f"Spider: [ ERROR ] __init__: {e}")

    def download_imgs_rec(self, count: int, node: Node | None = None):
        if not node:
            node = self.node
        self.download_imgs(node)

        if count > 0:
            count = count - 1
            if not node.childs:
                node.create_childs()
            for child in node.childs:
                count = self.download_imgs_rec(count, child)
                if not count:
                    break
        return count

    def download_imgs(self, node: Node | None = None):
        if not node:
            node = self.node
        if not node.html:
            if not node.create_html():
                print(f"Spider: [ ERROR ] Can't create HTML for {node.url}.")
                return

        print(f"Spider: [ INFO ] Downloading images from {node.url}...")
        try:
            os_makedir(self.img_dir + node.url, exist_ok=True)
        except Exception as e:
            print(f"Spider: [ ERROR ] os_makedir({self.img_dir + node.url}): {e}")
            return

        try:
            imgs = node.html.find_all("img")
            for img in imgs:
                url: str = img.get("src")
                if not url:
                    continue
                self.__download_img(node, url)

            styles = node.html.find_all(style=True)
            for style in styles:
                try:
                    content = style["style"]
                    if "background-image" in content:
                        url: str = content.split("url(")[-1].split(")")[0].strip("'\"")
                        self.__download_img(node, url)
                except:
                    print(f"Spider: [ ERROR ] download_imgs({node.url}): {e}")

            print(f"Spider: [ INFO ] {node.url} images download done.")
        except Exception as e:
            print(f"Spider: [ ERROR ] download_imgs({node.url}): {e}")

    def __download_img(self, node: Node, url: str):
        try:
            if not any(url.endswith(ext) for ext in self.img_ext):
                print(f"Spider: [ INFO ] {url}: {c_red}ext not supported{c_stop}")
                return

            if not url.startswith("http"):
                url = node.url + url
            with open(self.img_dir + node.url + "/" + os_path_basename(url), "wb") as f:
                f.write(requests_get(url).content)

            print(f"Spider: [ INFO ] {url}: {c_green}downloaded{c_stop}")
        except Exception as e:
            print(f"Spider: [ ERROR ] __download_img({node.url}, {url}): {e}")

    def load_gui(self, parent=None):
        if not os_path_exists(self.img_dir + self.root_node.url):
            print(f"Spider: [ ERROR ] load_gui: {self.img_dir + self.root_node.url} not found.")
            return
        try:
            self.gui = {
                "window": None,
                "tree": None,
            }
            self.gui["window"] = tkinter_Tk() if not parent else tkinter_Toplevel(parent)
            self.gui["window"].geometry("400x400")

            self.gui["tree"] = ttk_Treeview(self.gui["window"])
            self.gui["tree"].heading("#0", text=self.img_dir + self.root_node.url)
            self.gui["tree"].bind("<<TreeviewOpen>>", self.__on_treeview_open)
            self.gui["tree"].bind("<<TreeviewSelect>>", self.__on_treeview_select)

            self.__load_tree("", os_path_abspath(self.img_dir + self.root_node.url))
            self.gui["tree"].pack(fill="both", expand=True)

            self.gui["window"].title("S P I D E R")
            self.gui["window"].mainloop()
        except Exception as e:
            print(f"Spider: [ERROR] load_gui: {e}")

    def __load_tree(self, parent: str, parent_path: str):
        is_empty = True
        for item in os_scandir(parent_path):
            is_empty = False

            item_path = os_path_join(parent_path, item.name)
            if item.is_dir():
                new_item = self.gui["tree"].insert(parent, "end", text=item.name, values=[item_path], open=False)
                self.__load_tree(new_item, item_path)
            else:
                self.gui["tree"].insert(parent, "end", text=item.name, values=[item_path])

        if is_empty:
            self.gui["tree"].insert(parent, "end", text="this is empty")

    def __on_treeview_open(self, event):
        selected = self.gui["tree"].selection()
        if selected:
            values = self.gui["tree"].item(selected, "values")
            if len(values):
                selected_path = values[0]
                if os_path_isdir(selected_path):
                    self.gui["tree"].delete(*self.gui["tree"].get_children(selected))
                    self.__load_tree(selected, selected_path)

    def __on_treeview_select(self, event):
        selected = self.gui["tree"].selection()
        if selected:
            values = self.gui["tree"].item(selected, "values")
            if len(values):
                selected_path = values[0]
                if not os_path_isdir(selected_path) and self.__is_image_file(selected_path):
                    Scorpion(selected_path).show_metadata(True, self.gui["window"])

    def __is_image_file(self, img_path: str):
        ext = os_path_splitext(img_path)[1].lower()
        return ext in self.img_ext

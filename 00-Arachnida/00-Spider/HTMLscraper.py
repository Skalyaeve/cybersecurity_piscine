from os import makedirs
from os.path import basename
from requests import get
from bs4 import BeautifulSoup
from urllib.parse import urlparse

cGreen = "\033[92m"
cRed = "\033[91m"
cStop = "\033[0m"


class Node:
    def __init__(self, base_url: str, url: str, parent=None):
        if url[-1] == "/":
            url = url[:-1]
        self.base_url = base_url
        self.url = url
        self.html = None
        self.parsed = False
        self.parent = parent
        self.childs = None

    def create_html(self):
        try:
            response = get(self.url)
            try:
                self.html = BeautifulSoup(response.text, "html.parser")
                return True
            except Exception as e:
                print("HTMLscraper: [ERROR] BeautifulSoup(" + response.text + ", 'html.parser'):", e)
                return False
        except Exception as e:
            print("HTMLscraper: [ERROR] get(" + self.url + "):", e)
            return False

    def create_childs(self):
        try:
            if not self.html:
                print("\nHTMLscraper: can't create childs for " + self.url + ", html not created\n")
                return
            print("\nHTMLscraper: creating child links of", self.url)
            root_node = self.parent
            while root_node and root_node.parent:
                root_node = root_node.parent
            childs = []
            links = self.html.find_all("a")
            for link in links:
                already_in = False
                url: str = link.get("href")
                if (not url) or url[0] == "#":
                    continue
                if url[0] == "/":
                    url = self.base_url + url
                if url[-1] == "/":
                    url = url[:-1]
                if not url.startswith(self.base_url):
                    continue
                if url == self.url:
                    continue
                for child in childs:
                    if child.url == url:
                        already_in = True
                        break
                if already_in:
                    continue
                if self.__link_already_exist(url, root_node):
                    continue
                print(url)
                new_node = Node(self.base_url, url, self)
                childs.append(new_node)
            self.childs = childs
            print("HTMLscraper: child links of", self.url + cGreen + f" created" + cStop + f" ({len(childs)})\n")
        except Exception as e:
            print("HTMLscraper: [ERROR] create_childs():", e)

    def __link_already_exist(self, url: str, node):
        if not node:
            return False
        if url == node.url:
            return True
        if node.childs:
            for child in node.childs:
                if node.__link_already_exist(url, child):
                    return True
        return False


class HTMLscraper:
    def __init__(self, url: str, img_dir: str = "./", verbose: bool = True):
        try:
            self.img_ext = (".jpeg", ".jpg", ".png", ".gif", ".bmp")
            self.img_dir = img_dir if img_dir else "./"
            self.verbose = verbose
            self.root_node = Node(self.__removeUri(url), url)
            self.node = self.root_node
        except Exception as e:
            print("HTMLscraper: [ERROR] __init__:", e)
            self.root_node = None
            self.node = None

    def __removeUri(self, url: str):
        parsed_url = urlparse(url)
        return parsed_url.scheme + "://" + parsed_url.netloc

    def __download_img(self, node: Node, url: str):
        try:
            if not any(url.endswith(ext) for ext in self.img_ext):
                if self.verbose:
                    print(url, cRed + "ext not supported" + cStop)
                return
            if not url.startswith("http"):
                url = node.url + url
            response = get(url)
            imgName = basename(url)
            with open(self.img_dir + node.url.split("://", 2)[1] + "/" + imgName, "wb") as f:
                f.write(response.content)
            if self.verbose:
                print(url, cGreen + "downloaded" + cStop)
        except Exception as e:
            if self.verbose:
                print(cRed + "Download failed" + cStop + " for " + url + ", " + e)

    def download_imgs(self, node: Node | None = None):
        if not node:
            node = self.node
        if not node.html:
            if not node.create_html():
                print("HTMLscraper: [ERROR] can't create HTML for", node.url)
                return
        if self.verbose:
            print("HTMLscraper: Downloading images from " + node.url + "...")
        try:
            makedirs(self.img_dir + node.url.split("://", 2)[1], exist_ok=True)
        except Exception as e:
            print(f"HTMLscraper: [ERROR] can't makedirs({self.img_dir + node.url.split('://', 2)[1]}), ", e)
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
                    styleContent = style["style"]
                    if "background-image" in styleContent:
                        url: str = styleContent.split("url(")[-1].split(")")[0].strip("'\"")
                        self.__download_img(node, url)
                except:
                    continue
            if self.verbose:
                print("HTMLscraper: " + node.url + " images download done")
        except Exception as e:
            print("HTMLscraper: [ERROR] download_imgs():", e)

    def download_imgs_rec(self, count: int, node: Node | None = None):
        try:
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
        except Exception as e:
            print("HTMLscraper: [ERROR] download_imgs_rec():", e)

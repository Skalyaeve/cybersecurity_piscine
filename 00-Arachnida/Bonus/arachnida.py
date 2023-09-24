from Spider import Spider

if __name__ == "__main__":
    print("----------------- A R A C H N I D A -----------------")
    print("./spider.py --help for more information.")
    print("Remove SCHEME from URL to just open corresponding local files.")
    while True:
        params = input("\nSpider input: ").split()

        url = None
        recursive = False
        recursive_depth = 5
        path = "data/"
        try:
            i = 0
            while i < len(params):
                param = params[i]
                if param == "-r":
                    recursive = True
                elif param == "-l":
                    i += 1
                    recursive_depth = int(params[i])
                elif param == "-p":
                    i += 1
                    path = params[i]
                elif not url:
                    url = param
                else:
                    raise Exception("invalid arguments")
                i += 1
        except Exception as e:
            print(f"arachnida: [ERROR] __main__: {e}")
            continue

        if not url:
            print("arachnida: [ERROR] __main__: invalid url, expecting SCHEME://URL or just URL")
            continue
        if (url == "https://") or (url == "http://"):
            print("arachnida: [ERROR] __main__: invalid url, expecting SCHEME://URL or just URL")
            continue

        if not path:
            print("arachnida: [ERROR] invalid path.")
            continue
        if not path[-1] == "/":
            path = path + "/"

        worker = Spider(url, path)
        if (not url.startswith("http://")) and (not url.startswith("https://")):
            worker.load_gui()
            continue

        if recursive:
            worker.download_imgs_rec(recursive_depth)
        else:
            worker.download_imgs()
        worker.load_gui()
    print("-----------------------------------------------------")

from argparse import ArgumentParser

from Spider import Spider

if __name__ == "__main__":
    parser = ArgumentParser(description="This script downloads all images from a web page.")
    parser.add_argument("url", help="The URL of the web page you want to download images.")
    parser.add_argument("-r", action="store_true", help="Download images recursively.")
    parser.add_argument("-l", metavar="", type=int, default=5, help="Maximum depth level of the recursive download.")
    parser.add_argument("-p", metavar="", default="data/", help="Path where the downloaded files will be saved.")
    args = parser.parse_args()

    if not args.url:
        print("__main__: [ERROR] invalid url, expecting SCHEME://URL")
        exit(1)
    if (not args.url.startswith("http://")) and (not args.url.startswith("https://")):
        print("__main__: [ERROR] invalid url, expecting SCHEME://URL")
        exit(1)
    if (args.url == "https://") or (args.url == "http://"):
        print("__main__: [ERROR] invalid url, expecting SCHEME://URL")
        exit(1)

    if not args.p:
        print("__main__: [ERROR] invalid path.")
        exit(1)
    if not args.p[-1] == "/":
        args.p = args.p + "/"

    print("-------------------- S P I D E R --------------------")
    worker = Spider(args.url, args.p)
    if args.r:
        worker.download_imgs_rec(args.l)
    else:
        worker.download_imgs()
    print("-----------------------------------------------------")

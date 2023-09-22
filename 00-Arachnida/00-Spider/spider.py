from argparse import ArgumentParser

from HTMLscraper import HTMLscraper
from TkTreeViewer import TkTreeViewer


if __name__ == "__main__":
    parser = ArgumentParser(description="This script downloads all images from a web page")
    parser.add_argument("url", help="The URL of the web page you want to download images")
    parser.add_argument("-r", action="store_true", help="Download images recursively")
    parser.add_argument("-l", type=int, default=5, help="Maximum depth level of the recursive download")
    parser.add_argument("-p", default="./data/", help="Path where the downloaded files will be saved")
    parser.add_argument("-g", action="store_true", help="Show the graphic interface")
    parser.add_argument("-G", action="store_true", help="Show the graphic interface, no download, <url> become local path to display")
    args = parser.parse_args()

    if args.G:
        try:
            TkTreeViewer(args.url)
            exit(0)
        except Exception as e:
            print("spider: [ERROR] TkDirTree()", e)
            exit(1)

    if (not args.url.startswith("http://")) and (not args.url.startswith("https://")):
        print("spider: [ERROR] invalid url, expected: SCHEME://URL")
        exit(1)
    if args.url == "https://" or args.url == "http://":
        print("spider: [ERROR] invalid url, expected: SCHEME://URL")
        exit(1)
    if not args.p[-1] == "/":
        args.p = args.p + "/"

    scraper = HTMLscraper(args.url, args.p)
    if args.r:
        scraper.download_imgs_rec(args.l)
    else:
        scraper.download_imgs()

    if args.g:
        try:
            TkTreeViewer(args.p + args.url.split("://", 2)[1])
        except Exception as e:
            print("spider: [ERROR] TkDirTree()", e)
            exit(1)

from argparse import ArgumentParser

from Scorpion import Scorpion

c_red = "\033[91m"
c_stop = "\033[0m"

if __name__ == "__main__":
    parser = ArgumentParser(description="Display image metadata.")
    parser.add_argument("images", type=str, nargs="+", help="An image to analyze.")
    parser.add_argument("-m", metavar="TAG=VALUE", help="Modify specific EXIF data.")
    parser.add_argument("-d", metavar="TAG", help="Delete specific EXIF data.")
    args = parser.parse_args()

    allowedExt = (".jpeg", ".jpg", ".png", ".gif", ".bmp")
    if args.m:
        key, value = args.m.split("=", 2)
    worker = Scorpion()

    for img_path in args.images:
        if not any(img_path.endswith(ext) for ext in allowedExt):
            print(f"__main__: [ERROR] {img_path}:", c_red + "extension not supported" + c_stop)
            continue

        worker.load_data(img_path)
        if args.d:
            worker.modify_metadata(args.d, None)
        if args.m:
            worker.modify_metadata(key, value)
        worker.show_metadata()

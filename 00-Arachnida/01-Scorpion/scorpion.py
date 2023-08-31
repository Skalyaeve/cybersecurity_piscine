from argparse import ArgumentParser

from ExifManager import ExifManager

c_red = "\033[91m"
c_stop = "\033[0m"

if __name__ == "__main__":
    parser = ArgumentParser(description="Display image metadata")
    parser.add_argument("images", type=str, nargs="+", help="an image to analyze")
    parser.add_argument("-m", metavar="TAG=VALUE", help="modify specific EXIF data (e.g., -m TAG=VALUE)")
    parser.add_argument("-d", metavar="TAG", help="delete specific EXIF data (e.g., -d TAG)")
    parser.add_argument("-g", action="store_true", help="Show the graphic interface")
    args = parser.parse_args()

    allowedExt = (".jpeg", ".jpg", ".png", ".gif", ".bmp")
    for img_path in args.images:
        if not any(img_path.endswith(ext) for ext in allowedExt):
            print(f"scorpion: [ERROR] {img_path}:", c_red + "extension not supported" + c_stop)
            continue

        manager = ExifManager(img_path)
        if args.d:
            manager.delete(args.d)
        if args.m:
            try:
                key, value = args.m.split("=", 2)
                manager.modify(key, value)
            except:
                print(f"scorpion: [ERROR] invalid input for -m: {args.m}")
        manager.show(args.g)

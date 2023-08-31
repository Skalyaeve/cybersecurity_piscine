from argparse import ArgumentParser
from os.path import exists
from qrcode import make
from tkinter import Tk, Frame, Button, Label, font as tkFont
from tkinter.font import Font as tkFont
from PIL.ImageTk import PhotoImage

from FtHOTP import FtHOTP


def load_tk(value: str, generateQR: bool):
    try:
        window = Tk()
        window.title("ft_otp")
        label_frame = Frame(window, padx=10, pady=10)
        label_frame.pack()
        label = Label(label_frame)

        def display_code():
            code = FtHOTP(value).ft_hotp()
            if generateQR:
                qrcode = make(code)
                image = PhotoImage(qrcode)
                label.config(image=image)
                label.image = image
            else:
                label.config(
                    text=code,
                    font=tkFont(family="Arial", size=17, weight="bold"),
                )
            label.pack()

        button = Button(
            window,
            text="Generate",
            command=display_code,
            font=tkFont(family="Arial", size=12, weight="bold"),
            height=2,
            relief="groove",
            borderwidth=5,
            highlightthickness=2,
        )
        button.pack(side="bottom")
        window.mainloop()
    except Exception as e:
        print(f"ft_otp: [ERROR] load_tk():", e)


def print_code(value: str, generateQR: bool):
    try:
        code = FtHOTP(value).ft_hotp()
        if not generateQR:
            print(code)
        else:
            qrcode = make(code)
            qrcode.save("./qrcode.png")
    except Exception as e:
        print(f"ft_otp: [ERROR] print_code():", e)


def xor(key: str, value: str):
    msg = ""
    for x in value:
        msg += chr(ord(x) ^ key)
    return msg


def is_hex(string: str):
    return all(x in "0123456789abcdefABCDEF" for x in string)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("-g", "--generate", help="Generate the key and store it")
    parser.add_argument("-k", "--key", help="Generate OTP using the key", action="store_true")
    parser.add_argument("-q", help="Generate QrCode for OTP", action="store_true")
    parser.add_argument("-i", help="Show the graphic interface", action="store_true")
    args = parser.parse_args()

    key_filename = "ft_otp.key"
    XOR_key = 42
    if not (args.generate or args.key):
        print("usage: ft_otp.py [-h] [-g GENERATE] [-k] [...]\n-g: Generate the key and store it\n-k: Generate OTP using the key")
        exit(1)
    if args.generate:
        if not is_hex(args.generate) or len(args.generate) != 64:
            print("ft_otp: [ERROR] Key must be 64 hexadecimal characters")
            exit(1)
        try:
            print(key_filename)
            with open("ft_otp.key", "w") as file:
                file.write(xor(XOR_key, args.generate))
                print("ft_otp: Key was successfully saved in", key_filename)
        except Exception as e:
            print("ft_otp: [ERROR]", e)
    if args.key:
        if not exists(key_filename):
            print("ft_otp: [ERROR] Please generate the key first")
            exit(1)
        try:
            with open(key_filename, "r") as file:
                value = file.read()
            value = xor(XOR_key, value)
            load_tk(value, args.q) if args.i else print_code(value, args.q)
        except Exception as e:
            print("ft_otp: [ERROR]", e)

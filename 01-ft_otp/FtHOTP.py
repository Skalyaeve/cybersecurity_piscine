from hashlib import sha1
from hmac import new as hmac
from time import time


class FtHOTP:
    def __init__(self, key: str, digits: int = 6):
        self.key = key.encode()
        self.digits = digits

    def hotp(self):
        counter = (int(time())).to_bytes(9, "big")
        hmac_sha1 = hmac(self.key, counter, sha1).digest()
        offset = hmac_sha1[-1] & 0xF
        p1 = (hmac_sha1[offset] & 0x7F) << 24
        p2 = (hmac_sha1[offset + 1] & 0xFF) << 16
        p3 = (hmac_sha1[offset + 2] & 0xFF) << 8
        p4 = hmac_sha1[offset + 3] & 0xFF
        code = p1 | p2 | p3 | p4
        otp = code % (10**self.digits)
        return str(otp).zfill(self.digits)

    def ft_hotp(self):
        counter = (int(time())).to_bytes(9, "big")
        hmac_sha1 = self.__hmac(counter)
        offset = hmac_sha1[-1] & 0xF
        p1 = (hmac_sha1[offset] & 0x7F) << 24
        p2 = (hmac_sha1[offset + 1] & 0xFF) << 16
        p3 = (hmac_sha1[offset + 2] & 0xFF) << 8
        p4 = hmac_sha1[offset + 3] & 0xFF
        code = p1 | p2 | p3 | p4
        otp = code % (10**self.digits)
        return str(otp).zfill(self.digits)

    def __hmac(self, counter: bytes):
        block_size = sha1().block_size
        if len(self.key) > block_size:
            self.key = sha1(self.key).digest()
        if len(self.key) < block_size:
            self.key = self.key + b"\x00" * (block_size - len(self.key))
        opad = bytes((x ^ 0x5C) for x in self.key)
        ipad = bytes((x ^ 0x36) for x in self.key)
        inner_hash = sha1(ipad + counter).digest()
        return sha1(opad + inner_hash).digest()

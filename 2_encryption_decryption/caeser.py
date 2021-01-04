from sys import stdin, argv, stderr
import time

def caeserify(c, shift):
    if c.isalpha():
        if c.islower():
            base = ord('a')
        else:
            base = ord('A')

        return chr(base + ((ord(c) - base) + shift) % 26)

    else:

        return c


def encode(s, shift):

    def shifter(c):
        return caeserify(c, shift)

    res = ""
    for c in s:
        cdash = caeserify(c, shift)
        res += cdash

    return res

if __name__ == "__main__":

    n = len(argv)
    if n != 3:
        print("usage: python3 caeser.py <mode> <shift>", file = stderr)
        exit(1)
    __, mode, shift = argv

    if mode not in ['e', 'd']:
        print("<mode> must be in {e, d}", file = stderr)
        exit(1)

    shift = int(shift)

    if mode == 'd':
        # encoding and decoding are the same, but with reverse rotation
        shift = -shift
        phrase = 'decode'
    else:
        phrase = 'encode'

    iptext = stdin.read()

    t1 = time.time()
    optext = encode(iptext, shift) 
    t2 = time.time()

    print(optext, end = "")

    print("Time required to", phrase, ":", t2 - t1, file = stderr)

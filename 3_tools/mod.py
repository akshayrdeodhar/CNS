

from sys import argv, stderr


if __name__ == "__main__":

    n = len(argv)

    if n != 3:
        print("usage: python3 mod.py <num1> <num2>", file = stderr)
        exit(1)


    __, a, b = argv
    a = int(a)
    b = int(b)

    print("{0} % {1} = {2}".format(a, b, a % b))

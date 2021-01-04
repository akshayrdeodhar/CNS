from sys import argv, stderr

def gcd(a, b):
    small = min(a, b)
    big = max(a, b)

    if small == 0:
        return big

    return gcd(big % small, small)


if __name__ == "__main__":

    n = len(argv)

    if n != 3:
        print("usage: python3 gcd.py <num1> <num2>", file = stderr)
        exit(1)


    __, a, b = argv
    a = int(a)
    b = int(b)

    print("GCD({0}, {1}) = {2} ".format(a, b, gcd(a, b)))

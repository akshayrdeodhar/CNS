from sys import argv, stderr


if __name__ == "__main__":

    n = len(argv)

    if n != 4:
        print("usage: python3 mod.py <num1> <dir> <bits>", file = stderr)
        exit(1)


    __, n, direction, bits = argv
    n = int(n)
    bits = int(bits)

    if direction not in ['L', 'R']:
        print("Invalid direction {}".format(direction))


    if direction == 'L':
        print("{0} << {1} = {2}".format(n, bits, n << bits))
    else:
        print("{0} >> {1} = {2}".format(n, bits, n >> bits))

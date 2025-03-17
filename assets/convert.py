tab: list[list[int]] = [[0 for _ in range(41)] for _ in range(19)]

with open("ability.txt", 'r') as inp:
    for line in inp:
        if line[0] in ('#', ' ', '\n'):
            continue

        yx, val = line.split(':')
        y, x = tuple(map(int, yx.split(', ')))
        val = int(val[3:], 16)

        tab[y][x] = val
print(*tab, sep='\n')
with open("ability_", 'w') as out:
    for y, row in enumerate(tab):
        for x, val in enumerate(row):

            h = ''.join(map(lambda c: chr(int(c, 16)) ,('0' * 16 + hex(val)[2:])[-16:]))[::2]

            out.write(h)

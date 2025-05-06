tab: list[list[int]] = [[0 for _ in range(40)] for _ in range(18)]

with open("ability.txt", 'r') as inp:
    for line in inp:
        if line[0] in ('#', ' ', '\n'):
            continue

        yx, val = line.split(':')
        y, x = tuple(map(int, yx.split(', ')))
        val = int(val[3:], 16)

        tab[y-1][x-1] = val

print(*tab, sep='\n')
with open("ability", 'wb') as out:
    for y, row in enumerate(tab):
        for x, val in enumerate(row):
            padded_hex = ('0' * 16 + hex(val)[2:])[-16:]
            h = bytearray(map(lambda c: int(c, 16), (padded_hex[2*i:2*i+2] for i in range(8))))
            print(list(map(lambda c: int(c, 16), (padded_hex[2*i:2*i+2] for i in range(8)))))
            
            out.write(h)

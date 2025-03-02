from PIL import Image

im1 = Image.open('_roaster.png')
im2 = Image.open('empty_grid.png')
assert(im1.height == im2.height and im1.width == im2.width)

new = Image.new('RGBA', (im1.width, im1.height), (0, 0, 0, 0))

def sub(t1: tuple, t2: tuple) -> tuple:
    return tuple(x1 - x2 for x1, x2 in zip(t1, t2, strict=True))

for x in range(im1.width):
    for y in range(im1.height):
        px1 = im1.getpixel((x, y))
        px2 = im2.getpixel((x, y))
        diff = sub(px1, px2)
        if diff != (0, 0, 0, 0):
            new.putpixel((x, y), diff)


new.save('outpout.png')
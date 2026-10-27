from PIL import Image
import struct

im = Image.open('Logo UAN.png').convert('RGB')
im_c = im.crop((4, 140, 619, 323))
w,h = im_c.size
maxw = 420
if w > maxw:
    ratio = maxw/w
    im_c = im_c.resize((int(w*ratio), int(h*ratio)), Image.LANCZOS)
    w,h = im_c.size

data = bytearray()
for y in range(h):
    for x in range(w):
        r,g,b = im_c.getpixel((x,y))
        rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3)
        data += struct.pack('>H', rgb565)

out_path = 'Archivo/LogoUAN.h'
with open(out_path, 'w') as f:
    f.write('#pragma once\n\n')
    f.write('#include <Arduino.h>\n\n')
    f.write(f'#define LOGO_UAN_W {w}\n')
    f.write(f'#define LOGO_UAN_H {h}\n\n')
    f.write('static const uint16_t logoUAN[] PROGMEM = {\n')
    for i in range(0, len(data), 32):
        chunk = data[i:i+32]
        f.write('  ')
        vals = []
        for j in range(0, len(chunk), 2):
            val = (chunk[j] << 8) | chunk[j+1]
            vals.append(f'0x{val:04X}')
        f.write(','.join(vals) + ',\n')
    f.write('};\n')
print('ok', w, h)

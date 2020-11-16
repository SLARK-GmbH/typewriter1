from PIL import Image, ImageDraw, ImageFont
import os
import pprint
from pathlib import Path
import numpy as np
from color_constants import colors

img = Image.new('RGB', (1350, 330), color = (0, 0, 0))
#img = Image.new('RGB', (1350, 330), color = colors['rosybrown3'])

os.chdir('/home/me/python/binfonts')

fname = 'Coquin.ttf'
#fname = 'Abbasy Calligraphy Typeface.ttf'
fnameNoExt = Path(fname).stem

fsize = 16

fnt = ImageFont.truetype(fname, fsize)
 
 
#fnt = ImageFont.truetype('/opt/teamviewer/tv_bin/wine/share/wine/fonts/tahomabd.ttf', 16)
#fnt = ImageFont.truetype('/home/me/Arduino/libraries/SPIMemory/docs/_build/html/_static/fonts/Lato-Regular.ttf', 16)
#fnt = ImageFont.truetype('/home/me/Arduino/libraries/SPIMemory/docs/_build/html/_static/fonts/Lato-Regular.ttf', 32)
#fnt = ImageFont.truetype('/home/me/.local/share/fonts/Xerox Serif Narrow.ttf', 16)
#fnt = ImageFont.truetype('/usr/share/fonts/truetype/fonts-gujr-extra/Rekha.ttf', 32)

d = ImageDraw.Draw(img)

letter_strings = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\n\nthe quick brown fox jumps over the lazy dog\n\n1234567890\n\n!\"§$%&/()=?+*'#;:_,.-<>@{|}[]^`\\~"
letter_strings = '''THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\n\nthe quick brown fox jumps over the lazy dog\n\n1234567890\n\n!$%&/()=?+*'#;:_,.-<>@{|}[]^`\\~"'''

if True:        # insert 2 spaces after each letter to simplify the separation
    letter_strings = '\n'.join(['  '.join([l11 for l11 in l1.replace(' ','')]) for l1 in letter_strings.split('\n')])
    
#d.text((0,0), letter_strings, font=fnt, fill=(255, 255, 255))
d.text((0,0), letter_strings, font=fnt, fill=colors['white'])
 
#img.save('pil_text_font_tahomabd_16.png')
#img.save('Lato-Regular_16_1.png')
#img.save('Lato-Regular_32.png')
#img.save('Xerox_Serif_Narrow_16.png')
#img.save('Rekha_32.png')

img.save(fnameNoExt + '_%d_tf.png' % fsize)

# check size of fonts

width, height = img.size

print('img.size: ', width, height)

arr = np.array(img) # 640x480x4 array
arr[20, 30] # 4-vector, just like above

arr = img.load()
arr[20, 30] # tuple of 4 ints

#letter_strings = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\nthe quick brown fox jumps over the lazy dog\n1234567890\n!\"§$%&/()=?+*'#;:_,.-<>@{|}[]^`\\~".split('\n')
#letter_strings = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\nthe quick brown fox jumps over the lazy dog\n1234567890\n!$%&/()=?+*'#;:_,.-<>@{|}[]^`\\~".split('\n')

# ['@', '[', '\\', ']', '^', '`', '{', '|', '}', '~', '\x7f']
missing_chars_0x20_0x127 = [chr(i) for i in [i for i in range(0x20,0x80) if i not in sorted(list(set([ord(c1) for c1 in ' '.join(letter_strings)])))]]


range_down  = 0
range_up    = 19 * 2

lines = []  # [5, 20, 26, 41, 47, 59, 68, 83]

skip_empty = True

for j in range(height):
        if skip_empty and sum([arr[i,j][0] + arr[i,j][1] + arr[i,j][2] for i in range(width)]) > 0:
            lines.append(j)
            skip_empty = False

        if not skip_empty and sum([arr[i,j][0] + arr[i,j][1] + arr[i,j][2] for i in range(width)]) < 1:
            lines.append(j)
            skip_empty = True

print('lines')
pprint.pprint(lines)
# end of size checking

img.show()

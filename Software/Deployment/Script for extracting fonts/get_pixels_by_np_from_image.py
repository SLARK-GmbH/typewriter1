import numpy as np
from PIL import Image

import os, pprint

colors_of_grey = [' ', '1', '2', '3', '4', '5', '*']

"""
    # 0x000000, 0x080808, 0x282828, 0x606060, 0xa0a0a0, 0xb8b8b8, 0xffffff
   rgb3x8 -> rgb565 can be calculated using python lines:
   rgb3x8=0xfedcba
   and
   '%04x' % (((int(rgb3x8 % 0x100) & 0b11111000)<<8) + ((int((rgb3x8 / 0x100) % 0x100) & 0b11111100)<<3)+(int(rgb3x8 / 0x10000)>>3))
"""

rgb_colors_of_grey = [0x000000, 0x080808, 0x282828, 0x606060, 0xa0a0a0, 0xb8b8b8, 0xffffff]

rgb565_colors_of_grey = [0x0000, 0x0841, 0x2945, 0x630c, 0xa514, 0xbdd7, 0xffff]

j8 = 0

def get_grey_layers(mmggc):

    z0 = len(mmggc[0])

    mmggcg=[]

    for k in range(len(colors_of_grey)):
        mmggcg.append([])
        mmggcg[k] = '%02x ' % z0


    for k in range(len(colors_of_grey)):
        for i in range(z0):
            h = 0
            for j in range(16):
                    if mmggc[j][i] == colors_of_grey[k]:
                            h += 2 ** j

            #pprint.pprint(h)
            
            mmggcg[k] +=' '.join(['%02x' % (int(h/(256**k)) % 256) for k in range(2)]) + ' '

            #pprint.pprint(mmggcg[k])

    return mmggcg

def get_grey_layers_T(mmggc):

    mmggcT = np.array(mmggc).T.tolist()

    #pprint.pprint(mmggcT, width=234)
    
    z0 = len(mmggc[0])

    #pprint.pprint(z0)

    mmggcg=[]

    for k in range(len(colors_of_grey)):
        mmggcg.append([])
        mmggcg[k] = '%02x ' % z0

    jb = int(j8/8)

    for k in range(len(colors_of_grey)):
        for i in range(z0):
            h = 0
            #for j in range(16):
            for j in range(j8):
                    if mmggcT[i][j] == colors_of_grey[k]:
                            h += 2 ** j

            #pprint.pprint(h)
            
            mmggcg[k] += ' '.join(['%02x' % (int(h/(256**k)) % 256) for k in range(jb)]) + ' '

            #pprint.pprint(mmggcg[k])

    return mmggcg

os.chdir('/home/me/python/binfonts')

#img = Image.open('pil_text_font_tahomabd_16.png')
#img = Image.open('Lato-Regular_16.png')
img = Image.open('Lato-Regular_16_1.png')
#img = Image.open('Lato-Regular_32.png')
#img = Image.open('Xerox_Serif_Narrow_16.png')
#img = Image.open('Rekha_32.png')

arr = np.array(img) # 640x480x4 array
arr[20, 30] # 4-vector, just like above

arr = img.load()
arr[20, 30] # tuple of 4 ints

#print(len(arr[20,30]))

#print(img.size)

width, height = img.size

print('img.size: ', width, height)

letter_strings = '''THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\nthe quick brown fox jumps over the lazy dog\n1234567890\n!\"§$%&/()=?+*'#;:_,.-<>'''.split('\n')

letter_strings = "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG\nthe quick brown fox jumps over the lazy dog\n1234567890\n!\"§$%&/()=?+*'#;:_,.-<>@{|}[]^`\\~".split('\n')

# ['@', '[', '\\', ']', '^', '`', '{', '|', '}', '~', '\x7f']
missing_chars_0x20_0x127 = [chr(i) for i in [i for i in range(0x20,0x80) if i not in sorted(list(set([ord(c1) for c1 in ' '.join(letter_strings)])))]]


range_down  = 0
range_up    = 19 * 2

if False:
    '''[1, 10, 12, 21, 24, 32, 39, 50, 52, 61, 64, 69, 73, 82, 84, 93, 100, 108, 111, 119, 122, 132, 134, 149, 151, 160, 168, 175, 177, 187, 189, 199, 205,
210, 214, 223, 226, 236, 240, 248, 250, 258, 266, 276, 278, 288, 290, 298, 300, 308, 316, 325, 327, 336, 339, 347, 354, 372, 374, 382, 384, 394, 401, 410,
413, 423, 426, 436]'''
    for i in range(range_down, range_up):
        for j in range(0, 38):
            print('0x_%02x_%02x_%02x ' % arr[j, i], end =" ") 
        print() 
    1/0

lines = []  # [5, 20, 26, 41, 47, 59, 68, 83]

skip_empty = True

for j in range(height):
        if skip_empty and sum([arr[i,j][0] + arr[i,j][1] + arr[i,j][2] for i in range(width)]) > 0:
            lines.append(j)
            skip_empty = False

        if not skip_empty and sum([arr[i,j][0] + arr[i,j][1] + arr[i,j][2] for i in range(width)]) < 1:
            lines.append(j)
            skip_empty = True

mc = []

print('lines')
pprint.pprint(lines)

while 8 * j8 < sorted([lines[i] - lines[i -1] for i in range(1, len(lines) + 1, 2)])[-1]:
	j8 += 1

j8 *= 8

if j8 > 32:
    j8 = 32
    
for kj in range(0, len(lines), 2):
    #print(lines[kj])

    range_down  = lines[kj] - 1
    range_up    = range_down + j8
	 
    vlines = []

    skip_empty = False

    for i in range(width):
        if i:
            if skip_empty and sum([arr[i,j][0] + arr[i,j][1] + arr[i,j][2] for j in range(range_down,range_up)]) < 1:
                vlines.append(i)
                skip_empty = False

            if not skip_empty and sum([arr[i,j][0] + arr[i,j][1] + arr[i,j][2] for j in range(range_down,range_up)]) > 1:
                vlines.append(i)
                skip_empty = True            

    #letter_string = ''.join('THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG'.split())
    letter_string = ''.join(letter_strings[int(kj/2)].split())


    # double apostroph is a char with not consecutive parts - 2 apostrophs
    if '"' in letter_string:
        idx_of_d_apostroph = letter_string.index('"')
        idxs_2_remove = vlines[idx_of_d_apostroph * 2 : idx_of_d_apostroph * 2 + 4][1:3]
        for i2r in idxs_2_remove:
            vlines.remove(i2r)

    # there is no gap between "L" and "A"          TODO - how to recognise such case? --> simply insert in text after each letter an additional space
    if False and 'LA' in letter_string:
        vlines.insert(57,361)
        vlines.insert(58,361)
            
    o_mb = []
    r_mb = []

    #pprint.pprint(vlines)

    for k in range(0, len(vlines), 2):
        o_colors = []
        r_colors = []
        
        #print(letter_string[int(k/2)])

        #if letter_string[int(k/2)] != 'r': continue

        if False and letter_string[int(k/2)] == 'L':
            print(letter_string[int(k/2)])
            print(vlines)
            print(vlines[k])

        for i in range(range_down, range_up):
            o_lb = []
            r_lb = []
            for j in range(vlines[k], vlines[k + 1]):
                #print('0x_%02x_%02x_%02x ' % arr[j, i], end =" ")
                o_colors.append('0x%02x%02x%02x' % arr[j, i])
                ctemp = int('0x%02x%02x%02x' % arr[j, i], 16)
                o_lb.append('0x%02x%02x%02x' % arr[j, i])
                if False:
                    if ctemp > 0xc0c0c0:
                        r_colors.append(0xffffff)
                        r_lb.append('*')
                    elif ctemp > 0xa0a0a0:
                        r_colors.append(0xc0c0c0)
                        r_lb.append('x')
                    elif ctemp > 0x606060:
                        r_colors.append(0x404040)
                        r_lb.append('+')
                    else:
                        if r_lb:
                            r_colors.append(0x0)
                            r_lb.append(' ')
                        else:
                            if ctemp:
                                r_colors.append(0x404040)
                                r_lb.append('+')
                            else:
                                r_colors.append(0x0)
                                r_lb.append(' ')
                else:
                    if   ctemp == 0x0:      r_lb.append(' ')    # 0x000000 -> 0x0000
                    elif ctemp < 0x101010:  r_lb.append('1')    # 0x080808 -> 0x0841
                    elif ctemp < 0x404040:  r_lb.append('2')    # 0x282828 -> 0x2945
                    elif ctemp < 0x808080:  r_lb.append('3')    # 0x606060 -> 0x630c
                    elif ctemp < 0xc0c0c0:  r_lb.append('4')    # 0xa0a0a0 -> 0xa514
                    elif ctemp < 0xf0f0f0:  r_lb.append('5')    # 0xb8b8b8 -> 0xbdd7
                    elif ctemp > 0xefefef:  r_lb.append('*')    # 0xffffff -> 0xffff
                    
            #print()
            #mb.append(''.join(lb))
            o_mb.append(o_lb)
            r_mb.append([' '] + r_lb)
        
        o_color_list = sorted(list(set(o_colors)))
        usage_o_colors = sorted([(c1, o_colors.count(c1)) for c1 in o_color_list if c1 not in ['0x000000', '0xffffff']], key = lambda x:x[0])
        #print()
        #print(len(o_color_list), o_color_list)
        #print(sorted(usage_o_colors, key=lambda x:x[1], reverse = True))
        r_color_list = sorted(list(set(r_colors)))
        #print(int(k/2))
        
        #print('letter_string')
        #print(len(letter_string))
        #print(letter_string[int(k/2)])

        #print(vlines[k], vlines[k+1])
        #print(len(r_color_list), r_color_list)
        #mc.append((letter_string[int(k/2)], len(o_color_list), o_color_list, o_mb, len(r_color_list), r_color_list, r_mb, get_grey_layers(r_mb)))
        #mc.append((letter_string[int(k/2)], r_mb, get_grey_layers_T(r_mb)))

        #pprint.pprint(mc, width=500)
        #print(len(mc[0]))
        #pprint.pprint(mc[0][2])

        aux = get_grey_layers_T(r_mb)

        aux_l = int(aux[0].split()[0],16)

        by=[0] * 32 * aux_l
        
        for i, t in enumerate([(n, b1) for n, b1 in enumerate([''.join([format(int(l11,16), '08b') for l11 in l1.split()]) for l1 in [' '.join([y11 for y11 in y1.split()[1:]]) for y1 in aux][1:]])]):
            for j, t1 in enumerate(t[1]):
                    if int(t1):
                            by[j] = by[j] + i + 1
        #print(by)
        by.insert(0, aux_l)

        mc.append((letter_string[int(k/2)], r_mb, aux, by))
        
        #print()
        o_mb = []
        r_mb = []

#pprint.pprint(sorted(mc, key=lambda x:x[0]), width = 550)

#pprint.pprint([(chr(ord(' ') + n), e1 - 231) for n, e1 in enumerate(np.cumsum([s1 + e1[1] for e1 in [(letter, dmc[letter][-1][0] * 7 * 4 + 7) for letter in [chr(ijk) for ijk in range(0x20,0x7f)]]]).tolist())])

max_length = sorted([len(m1[-2][0].split()) for m1 in mc], reverse=True)[0]

dmc = dict([(m1[0],   [m11 + ' '.join(['00'] * (max_length - len(m11.split()))) for m11 in m1[-2]] )for m1 in mc])

dmc = dict([(m1[0],   [bytearray([int(s1,16) for s1 in (m11 + ' '.join(['00'] * (max_length - len(m11.split())))).split()]) for m11 in reversed(m1[-2])] ) for m1 in mc])

dmc[' '] = [bytearray([8] + [0] * (max_length - 1))] * 7

#pprint.pprint([(chr(ord(' ') + n), e1 - 231) for n, e1 in enumerate(np.cumsum([e1[1] for e1 in [(letter, dmc[letter][-1][0] * 7 * 4 + 7) for letter in [chr(ijk) for ijk in range(0x20,0x7f)]]]).tolist())])

"""
by=[0]*544

for i, t in enumerate([(n, b1) for n, b1 in enumerate([''.join([format(int(l11,16), '08b') for l11 in l1.split()]) for l1 in [' '.join([y11 for y11 in y1.split()[1:]]) for y1 in y][1:]])]):
	print(i,t)
	for j, t1 in enumerate(t[1]):
		if int(t1):
			by[j] = by[j] + i + 1
"""


data_4_file=[]
	      
for i in range(6):
    l1 = []
    data_4_file.append(l1)

for letter in [chr(ijk) for ijk in range(0x20,0x80)]:
    if letter in dmc:
        for layer in range(0,6):
            data_4_file[layer] += dmc[letter][layer]
    else:
        for layer in range(0,6):
            data_4_file[layer] += dmc[' '][layer]

#pprint.pprint(data_4_file, width=500)
        
#fb = open('Xerox_Serif_Narrow_16_mg.bin', 'wb')
fb = open('Lato-Regular_16_mg_1.bin', 'wb')
#fb = open('Rekha_32_mg.bip', 'wb')
#fb = open('Lato-Regular_32_mg_1.bip', 'wb')

for layer in range(6):
    fb.write(bytearray(data_4_file[layer]))

fb.close()    


#pprint.pprint(mc)

dmci = []
#dmci += [m1[-1] + [0] * (max_length - len(m1[-1])) for m1 in mc]
for m1 in mc:
    dmci += m1[-1] + [0] * (max_length - len(m1[-1]))

data_4_file=[]

#fb = open('Lato-Regular_32_mg_2.bip', 'wb')
fb = open('Lato-Regular_16_mg_2.bin', 'wb')

fb.write(bytearray(dmci))

fb.close()   

print('max_length: ', max_length)

import os, pprint

c_file = 'usToDE'

os.system('avr-gcc -fverbose-asm -save-temps %s.c' % c_file)

a = []

f = open(c_file + '.s')

for line in f.readlines():
	#print(line)
	if 'byte' in line:
		a.append(int(line.split()[-1]))

f.close()

print(a)

f = open(c_file + '.bik', 'wb')

name = []

[name.append(ord(l1)) for l1 in c_file]

name += [0] * (0x20 - len(c_file))

f.write(bytearray(name + [(a1, 256 + a1)[a1 < 0] for a1 in a[0x20:]]))

f.close()

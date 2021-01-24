print('Usage: at # prompt, enter 6-digit hex code or nothing to quit')

while True:
    h = input('#')
    n = int(h.strip(), base=16)
    print('vec3(', end='')
    print(round((n >> 16)/0xFF, 3), end=', ')
    print(round(((n >> 8) & 0xFF)/0xFF, 3), end=', ')
    print(round((n & 0xFF)/0xFF, 3), end=')\n')

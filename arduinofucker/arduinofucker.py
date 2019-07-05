#!/usr/bin/env python3
from serial import Serial
import sys

port = sys.argv[1]

s = Serial('/dev/'+port, 9600, timeout=1)

while True:
    print("Menu:")
    print("1. Move Up")
    print("2. Move Down")
    print("3. Light on")
    print("4. Light off")
    print("5. Flip page")
    print("\n6. Exit")
    print("\n %: ", end = '')

    c = int(input())
    if c == 1:
        s.write(b'\x02\x01')
        back = s.read(100)
        print(back)
    if c == 2:
        s.write(b'\x02\x00')
        back = s.read(100)
        print(back)
    if c == 3:
        s.write(b'\x04\x01')
        back = s.read(100)
        print(back)
    if c == 4:
        s.write(b'\x04\x00') #C-C++ Reverenz 2nd edition
        back = s.read(100)
        print(back)
    if c == 5:
        s.write(b'\x08\x13')
        back = s.read(800)
        print(back)
    if c == 6:
        break

s.close()

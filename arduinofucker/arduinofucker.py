#!/usr/bin/env python3
from serial import Serial
import threading
import sys
import time

running = threading.Event()
running.set()

def thread_read(ser):
    while running.is_set():
        buf = ser.readline(1024)
        if buf != b'':
            print(buf)


port = sys.argv[1]

s = Serial(port, 9600, timeout=1)

th = threading.Thread(target=thread_read, args=(s, )) # I both love and hate python for this.
th.start()

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
    if c == 2:
        s.write(b'\x02\x00')
    if c == 3:
        s.write(b'\x04\x01')
    if c == 4:
        s.write(b'\x04\x00')
    if c == 5:
        print('Book Height (mm) = ', end='')
        h = int(input())
        if h > 255:
            print('Book Too Large')
        else:
            s.write(b'\x08'+bytes([h])) #C-C++ Reverenz 2nd edition
    if c == 6:
        running.clear()
        th.join()
        break
    time.sleep(2);

s.close()

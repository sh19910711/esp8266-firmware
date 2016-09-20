#!/usr/bin/env python3
import os
import sys
import argparse
import serial
import threading
import time


def read_handler():
    l = ""
    booted = False
    while True:
        try:
            b = serial_port.read()
            s = b.decode('ascii')
        except UnicodeDecodeError:
            pass
        else:
            if booted:
                print(s, end='')
            l += s
            if "\n" in l:
                if not booted and "firmware:" in l:
                    print(l)
                    booted = True
                elif ">>>stack>>>" in l:
                    print("*** stack dump!")
                    os._exit(1) # FIXME
                l = ""
                
            sys.stdout.flush()


def write_handler():
    while True:
        l = sys.stdin.readline().strip()
        print("*** sending '{}'".format(l))
        ch = serial_port.write(l.encode('ascii'))


def main():
    global serial_port

    parser = argparse.ArgumentParser()
    parser.add_argument("tty")
    args = parser.parse_args()

    print("*** opening", args.tty)
    serial_port = serial.Serial(args.tty, 115200, timeout=1)
    print("*** listening", args.tty)

    threading.Thread(target=write_handler, daemon=True).start()
    threading.Thread(target=read_handler, daemon=True).start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        serial_port.close()
        sys.exit()


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
import sys
import argparse
import serial
import threading
import time


def read_handler():
    while True:
        try:
            ch = s.read().decode('ascii')
        except UnicodeDecodeError:
            print("non-ascii character: ".format(hex(ch)))
        else:
            print(ch, end='')
            sys.stdout.flush()


def write_handler():
    while True:
        l = sys.stdin.readline().strip()
        print("*** sending '{}'".format(l))
        ch = s.write(l.encode('ascii'))


def main():
    global s

    parser = argparse.ArgumentParser()
    parser.add_argument("tty")
    args = parser.parse_args()

    print("*** opening", args.tty)
    s = serial.Serial(args.tty, 115200, timeout=1)
    print("*** listening", args.tty)

    threading.Thread(target=write_handler, daemon=True).start()
    threading.Thread(target=read_handler, daemon=True).start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        s.close()
        sys.exit()


if __name__ == '__main__':
    main()

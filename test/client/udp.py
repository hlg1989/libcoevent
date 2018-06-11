#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import socket
import sys

from tools import log


def main():
    # "main process"
    log.debug('Hello libcoevent UDP!')
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    msg = 'Hello, libcoevent from python UDP'
    # msg = 'quit'
    sock.sendto(msg.encode(), 0, ('127.0.0.1', 2333))
    log.debug('Data sent')
    return


if __name__ == '__main__':
    # sys.settrace(trace)
    ret = main()

    if ret is None:
        ret = 0
    sys.exit(ret)

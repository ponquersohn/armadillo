#!/usr/bin/env python3
"""Armadillo kernel module control tool."""

import argparse
import ctypes
import fcntl
import os
import sys

DEVICE = "/dev/armadillo_cdrv"
MAX_PASS_LEN = 15
MAX_PASS_LEN_TERM = MAX_PASS_LEN + 1


# Replicate Linux _IOC / _IO / _IOW macros exactly
def _IOC(direction, type_, nr, size):
    return (direction << 30) | (size << 16) | (type_ << 8) | nr

def _IO(type_, nr):        return _IOC(0, type_, nr, 0)
def _IOW(type_, nr, size): return _IOC(1, type_, nr, size)


# Mirror the C structs from command_ioctl.h — ctypes handles alignment/padding
class IoctlLock(ctypes.Structure):
    _fields_ = [("secret", ctypes.c_ubyte * MAX_PASS_LEN_TERM)]

class IoctlUnlock(ctypes.Structure):
    _fields_ = [("secret", ctypes.c_ubyte * MAX_PASS_LEN_TERM)]

class IoctlSetPidUnkillable(ctypes.Structure):
    _fields_ = [
        ("secret",     ctypes.c_ubyte * MAX_PASS_LEN_TERM),
        ("pid",        ctypes.c_uint),
        ("new_status", ctypes.c_ubyte),
    ]


IOCTL_MAGIC = 0x33

# _IOW size arg matches the original C macros
IOCTL_LOCK              = _IOW(IOCTL_MAGIC, 1, ctypes.sizeof(IoctlLock))
IOCTL_UNLOCK            = _IOW(IOCTL_MAGIC, 2, ctypes.sizeof(IoctlUnlock))
IOCTL_SET_PID_UNKILLABLE = _IOW(IOCTL_MAGIC, 3, ctypes.sizeof(ctypes.c_uint))
IOCTL_SET_DEBUG         = _IO(IOCTL_MAGIC, 4)


def open_device():
    try:
        return os.open(DEVICE, os.O_RDONLY)
    except PermissionError:
        sys.exit(f"Permission denied: {DEVICE} — run as root.")
    except FileNotFoundError:
        sys.exit(f"Device not found: {DEVICE} — is the module loaded?")


def encode_secret(password: str) -> ctypes.Array:
    raw = password.encode()[:MAX_PASS_LEN]
    buf = (ctypes.c_ubyte * MAX_PASS_LEN_TERM)()
    for i, b in enumerate(raw):
        buf[i] = b
    return buf


def do_lock(args):
    params = IoctlLock()
    params.secret = encode_secret(args.password)
    fd = open_device()
    try:
        fcntl.ioctl(fd, IOCTL_LOCK, bytearray(params))
        print("Locked.")
    except OSError as e:
        sys.exit(f"lock failed: {e}")
    finally:
        os.close(fd)


def do_unlock(args):
    params = IoctlUnlock()
    params.secret = encode_secret(args.password)
    fd = open_device()
    try:
        fcntl.ioctl(fd, IOCTL_UNLOCK, bytearray(params))
        print("Unlocked.")
    except OSError as e:
        sys.exit(f"unlock failed: {e}")
    finally:
        os.close(fd)


def do_set_unkillable(args):
    params = IoctlSetPidUnkillable()
    params.pid = args.pid
    params.new_status = 1 if args.state == "on" else 0
    fd = open_device()
    try:
        fcntl.ioctl(fd, IOCTL_SET_PID_UNKILLABLE, bytearray(params))
        print(f"pid: {args.pid}, new_status: {params.new_status}")
    except OSError as e:
        sys.exit(f"set_unkillable failed: {e}")
    finally:
        os.close(fd)


def do_set_debug(args):
    fd = open_device()
    try:
        fcntl.ioctl(fd, IOCTL_SET_DEBUG)
        print("Debug toggled.")
    except OSError as e:
        sys.exit(f"set_debug failed: {e}")
    finally:
        os.close(fd)


def main():
    parser = argparse.ArgumentParser(
        prog="armadillo",
        description="Armadillo kernel module control tool",
    )
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("lock", help="Lock the module with a password")
    p.add_argument("password", help=f"Password (max {MAX_PASS_LEN} chars)")
    p.set_defaults(func=do_lock)

    p = sub.add_parser("unlock", help="Unlock the module")
    p.add_argument("password", help="Password used when locking")
    p.set_defaults(func=do_unlock)

    p = sub.add_parser("set_unkillable", help="Protect/unprotect a PID from kill signals")
    p.add_argument("pid", type=int, help="Target process ID")
    p.add_argument("state", choices=["on", "off"], help="on = protect, off = unprotect")
    p.set_defaults(func=do_set_unkillable)

    p = sub.add_parser("set_debug", help="Toggle kernel debug output")
    p.set_defaults(func=do_set_debug)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()

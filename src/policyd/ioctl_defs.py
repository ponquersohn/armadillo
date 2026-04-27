"""Python mirror of src/module/command_ioctl.h policy interface.

Keep in sync with the C header. Layouts use native alignment (the C structs
in the kernel are not __packed__, so we match that).
"""
import ctypes
import struct

# --- from defines.h ---
ARMADILLO_POLICY_PATH_MAX = 512
ARMADILLO_POLICY_COMM_LEN = 16
ARMADILLO_DEVICE = "/dev/armadillo_cdrv"

# --- _IOC encoding, matches <asm-generic/ioctl.h> ---
_IOC_NRBITS   = 8
_IOC_TYPEBITS = 8
_IOC_SIZEBITS = 14
_IOC_DIRBITS  = 2

_IOC_NRSHIFT   = 0
_IOC_TYPESHIFT = _IOC_NRSHIFT + _IOC_NRBITS
_IOC_SIZESHIFT = _IOC_TYPESHIFT + _IOC_TYPEBITS
_IOC_DIRSHIFT  = _IOC_SIZESHIFT + _IOC_SIZEBITS

_IOC_NONE  = 0
_IOC_WRITE = 1
_IOC_READ  = 2

def _IOC(direction: int, type_: int, nr: int, size: int) -> int:
    return ((direction << _IOC_DIRSHIFT) |
            (type_     << _IOC_TYPESHIFT) |
            (nr        << _IOC_NRSHIFT) |
            (size      << _IOC_SIZESHIFT))

def _IO(t, n):      return _IOC(_IOC_NONE, t, n, 0)
def _IOR(t, n, s):  return _IOC(_IOC_READ, t, n, s)
def _IOW(t, n, s):  return _IOC(_IOC_WRITE, t, n, s)

ARMADILLO_IOCTL_MAGIC = 0x33


class PolicyRequest(ctypes.Structure):
    _fields_ = [
        ("id",   ctypes.c_uint64),
        ("pid",  ctypes.c_int32),
        ("ppid", ctypes.c_int32),
        ("uid",  ctypes.c_uint32),
        ("comm", ctypes.c_char * ARMADILLO_POLICY_COMM_LEN),
        ("path", ctypes.c_char * ARMADILLO_POLICY_PATH_MAX),
    ]


class PolicyReply(ctypes.Structure):
    _fields_ = [
        ("id",      ctypes.c_uint64),
        ("verdict", ctypes.c_uint32),
    ]


class PolicyConfig(ctypes.Structure):
    _fields_ = [
        ("timeout_ms",  ctypes.c_uint32),
        ("fail_closed", ctypes.c_uint32),
    ]


VERDICT_ALLOW = 1
VERDICT_DENY  = 2


IOCTL_POLICY_ATTACH     = _IO (ARMADILLO_IOCTL_MAGIC, 10)
IOCTL_POLICY_PULL       = _IOR(ARMADILLO_IOCTL_MAGIC, 11, ctypes.sizeof(PolicyRequest))
IOCTL_POLICY_REPLY      = _IOW(ARMADILLO_IOCTL_MAGIC, 12, ctypes.sizeof(PolicyReply))
IOCTL_POLICY_SET_CONFIG = _IOW(ARMADILLO_IOCTL_MAGIC, 13, ctypes.sizeof(PolicyConfig))
IOCTL_POLICY_GET_CONFIG = _IOR(ARMADILLO_IOCTL_MAGIC, 14, ctypes.sizeof(PolicyConfig))

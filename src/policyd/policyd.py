#!/usr/bin/env python3
"""Armadillo policy daemon — mediates execve via /dev/armadillo_cdrv."""
import argparse
import ctypes
import errno
import fcntl
import os
import signal
import sys

from ioctl_defs import (
    ARMADILLO_DEVICE,
    IOCTL_POLICY_ATTACH,
    IOCTL_POLICY_PULL,
    IOCTL_POLICY_REPLY,
    IOCTL_POLICY_SET_CONFIG,
    PolicyConfig,
    PolicyReply,
    PolicyRequest,
    VERDICT_ALLOW,
    VERDICT_DENY,
)


running = True


def _stop(_signum, _frame):
    global running
    running = False


def decide(req: PolicyRequest) -> int:
    """Stub policy — v1 allows everything and logs."""
    path = req.path.decode("utf-8", errors="replace")
    comm = req.comm.decode("utf-8", errors="replace")
    print(f"[policyd] ALLOW id={req.id} pid={req.pid} uid={req.uid} "
          f"comm={comm} path={path}", flush=True)
    return VERDICT_ALLOW


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--timeout-ms", type=int, default=2000,
                    help="kernel wait timeout per request (0 = forever)")
    ap.add_argument("--fail-closed", action="store_true",
                    help="deny on timeout / daemon detach")
    ap.add_argument("--device", default=ARMADILLO_DEVICE)
    args = ap.parse_args()

    fd = os.open(args.device, os.O_RDWR)
    try:
        fcntl.ioctl(fd, IOCTL_POLICY_ATTACH)
        cfg = PolicyConfig(timeout_ms=args.timeout_ms,
                           fail_closed=1 if args.fail_closed else 0)
        fcntl.ioctl(fd, IOCTL_POLICY_SET_CONFIG, bytes(cfg))
        print(f"[policyd] attached to {args.device}: "
              f"timeout_ms={args.timeout_ms} fail_closed={args.fail_closed}",
              flush=True)

        signal.signal(signal.SIGINT, _stop)
        signal.signal(signal.SIGTERM, _stop)

        req = PolicyRequest()
        while running:
            try:
                fcntl.ioctl(fd, IOCTL_POLICY_PULL, req, True)
            except InterruptedError:
                continue
            except OSError as e:
                if e.errno == errno.EINTR:
                    continue
                if e.errno == errno.ESHUTDOWN:
                    print("[policyd] kernel shutting down", flush=True)
                    break
                raise

            verdict = decide(req)
            reply = PolicyReply(id=req.id, verdict=verdict)
            try:
                fcntl.ioctl(fd, IOCTL_POLICY_REPLY, bytes(reply))
            except OSError as e:
                if e.errno == errno.ENOENT:
                    # Request was abandoned (timeout) — ignore.
                    continue
                raise
    finally:
        # On graceful shutdown, restore fail-open so the system stays usable.
        # A crash or SIGKILL skips this path — fail-closed stays in effect.
        try:
            safe = PolicyConfig(timeout_ms=args.timeout_ms, fail_closed=0)
            fcntl.ioctl(fd, IOCTL_POLICY_SET_CONFIG, bytes(safe))
        except OSError:
            pass
        try:
            os.close(fd)
        except OSError:
            pass
        print("[policyd] exiting", flush=True)


if __name__ == "__main__":
    sys.exit(main() or 0)

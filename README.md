# Armadillo

A Linux kernel module that hardens a running system against accidental or
malicious root damage. Originally written for LS 2020 as a weekend project,
now updated for modern kernels and extended with a userspace policy daemon.

Armadillo ships as:

- `armadillo.ko` — the kernel module (syscall hooks via ftrace, ioctl control
  device at `/dev/armadillo_cdrv`)
- `interface` — a small C CLI for sending ioctls
- `tools/armadillo.py` — the same control surface in Python
- `policyd` — a Python daemon that mediates `execve` from userspace

## Features

- **Unkillable PIDs** — mark a PID immune to `SIGKILL` / `SIGTERM`.
- **Locked configuration** — once locked with a password, config changes and
  module unload (including `rmmod --force`) are refused until unlocked. The
  password is held in memory AES-encrypted under a random key.
- **ftrace syscall hooking** — currently hooks `ioctl` to gate `chattr`-style
  attribute changes.
- **Policy mediation for `execve`** — the kernel parks each `execve` and asks
  a userspace daemon for an ALLOW / DENY verdict, with a configurable
  `timeout_ms` and `fail_closed` policy for when the daemon is absent or slow.

## Build

Requires kernel headers for the running kernel.

```
# RHEL/CentOS/Fedora
dnf install kernel-devel-$(uname -r)

# Debian/Ubuntu
apt install linux-headers-$(uname -r)

make
```

This builds `src/module/armadillo.ko`, `src/userspace/interface`, and the
`policyd` Python package.

## Usage

### Load / unload

```
insmod src/module/armadillo.ko
rmmod armadillo
```

### Lock / unlock the configuration

```
./src/interface lock    <password>
./src/interface unlock  <password>
```

Or using the Python tool:

```
sudo tools/armadillo.py lock   <password>
sudo tools/armadillo.py unlock <password>
```

While locked, `rmmod` and `rmmod --force` both fail and configuration ioctls
are rejected.

### Protect a PID from kill

```
sleep 3600 & PID=$!
./src/interface set_unkillable $PID on
kill -9 $PID          # no effect
./src/interface set_unkillable $PID off
kill -9 $PID          # terminates
```

### Run the execve policy daemon

```
sudo src/policyd/policyd.py --timeout-ms 2000            # fail-open on timeout
sudo src/policyd/policyd.py --timeout-ms 2000 --fail-closed
```

The stub policy in `policyd.py` logs and allows every `execve`. Replace
`decide()` with your own logic to enforce real rules. On graceful shutdown the
daemon restores fail-open so the system stays usable; a crash with
`--fail-closed` set will leave the kernel denying execs until a daemon
reattaches.

## Development

A `virtme-ng` helper boots a throwaway VM with the project mounted
read-write, so kernel crashes are cheap:

```
make vm                       # interactive shell
tools/vm.sh --exec tools/test-module.sh    # module smoke test
tools/vm.sh --exec tools/test-policy.sh    # execve mediation smoke test
```

## Roadmap

Must-have:
- block remount-rw of `/`
- protect specified files from `rm`
- disable selected syscalls for root
- delayed arming (arm N seconds after boot)

Nice-to-have:
- obfuscate the module (encrypted text, decrypted in memory)
- remote control channel over sockets

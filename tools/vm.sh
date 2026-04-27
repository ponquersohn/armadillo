#!/usr/bin/env bash
# Launch a virtme-ng VM for kernel module development.
# The project dir is mounted read-write at the same path inside the VM.
# On VM crash/panic, just re-run this script — no snapshot management needed.
#
# Usage:
#   ./tools/vm.sh            — interactive shell
#   ./tools/vm.sh --exec CMD — run a command and exit (e.g. for CI)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Ensure vmlinuz is readable (needs to be done once with sudo)
VMLINUZ="/boot/vmlinuz-$(uname -r)"
if [ ! -r "$VMLINUZ" ]; then
    echo "Error: $VMLINUZ is not readable."
    echo "Fix with: sudo chmod 644 $VMLINUZ"
    exit 1
fi

# Build the module before launching if .ko is missing or sources are newer
KO="$PROJECT_ROOT/src/module/armadillo.ko"
if [ ! -f "$KO" ] || find "$PROJECT_ROOT/src/module" -name "*.c" -newer "$KO" | grep -q .; then
    echo "[vm.sh] Building module..."
    make -C "$PROJECT_ROOT/src/module"
fi

echo "[vm.sh] Starting virtme-ng VM (kernel: $(uname -r))"
echo "[vm.sh] Project dir available at: $PROJECT_ROOT"
echo "[vm.sh] To load module:  cd src/module && sudo insmod armadillo.ko"
echo "[vm.sh] To unload:       sudo rmmod armadillo"
echo "[vm.sh] Crash/hang?      Ctrl-a x  (QEMU escape), then re-run ./tools/vm.sh"
echo ""

virtme-ng --run \
    --rwdir "$PROJECT_ROOT" \
    --cpus 2 \
    --memory 1024 \
    "$@"

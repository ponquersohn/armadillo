#!/usr/bin/env bash
# Smoke-test for armadillo.ko — intended to run inside a virtme-ng VM.
# Usage: virtme-ng --run --rwdir <project> --exec tools/test-module.sh

PROJECT=/home/ponq/dev/armadillo
KO=$PROJECT/src/module/armadillo.ko
IFACE=$PROJECT/src/userspace/interface.static

PASS=0
FAIL=0

pass() { echo "  PASS: $*"; PASS=$((PASS+1)); }
fail() { echo "  FAIL: $*"; FAIL=$((FAIL+1)); }
section() { echo ""; echo "=== $* ==="; }

echo "============================================"
echo " Armadillo module smoke-test"
echo " Kernel: $(uname -r)"
echo "============================================"

# ---- load ----
section "Load module"
insmod "$KO"
RC=$?
[ $RC -eq 0 ] && pass "insmod returned 0" || fail "insmod returned $RC"

echo ""
echo "--- lsmod ---"
lsmod | grep armadillo

echo ""
echo "--- dmesg (armadillo) ---"
dmesg | grep -i armadillo | tail -20

# ---- set_unkillable ----
section "set_unkillable"
sleep 3600 &
PID=$!
echo "  Launched sleep 3600 (PID=$PID)"

"$IFACE" set_unkillable $PID on
echo "  Sending SIGKILL to PID $PID (should be blocked)..."
kill -9 $PID 2>&1 || true
sleep 0.4

if kill -0 $PID 2>/dev/null; then
    pass "PID $PID still alive after SIGKILL (unkillable works)"
else
    fail "PID $PID was killed — unkillable flag not working"
fi

"$IFACE" set_unkillable $PID off
kill -9 $PID 2>&1 || true
sleep 0.4

if kill -0 $PID 2>/dev/null; then
    fail "PID $PID still alive after clearing unkillable flag"
else
    pass "PID $PID killed after clearing unkillable flag"
fi

# ---- lock / rmmod prevention ----
section "Lock and rmmod prevention"
"$IFACE" lock secretpass
echo "  Locked with password 'secretpass'"

echo "  Trying rmmod while locked..."
if rmmod armadillo 2>/dev/null; then
    fail "rmmod succeeded while locked!"
else
    pass "rmmod blocked while locked"
fi

echo "  Trying rmmod --force while locked..."
if rmmod --force armadillo 2>/dev/null; then
    fail "rmmod --force succeeded while locked!"
else
    pass "rmmod --force blocked while locked"
fi

# ---- unlock ----
section "Unlock"
"$IFACE" unlock secretpass
echo "  Unlocked"

# ---- unload ----
section "Unload module"
rmmod armadillo
RC=$?
[ $RC -eq 0 ] && pass "rmmod returned 0" || fail "rmmod returned $RC"

if lsmod | grep -q armadillo; then
    fail "module still in lsmod after rmmod"
else
    pass "module gone from lsmod"
fi

echo ""
echo "--- dmesg tail (armadillo) ---"
dmesg | grep -i armadillo | tail -20

# ---- summary ----
echo ""
echo "============================================"
echo " Results: $PASS passed, $FAIL failed"
echo "============================================"
[ $FAIL -eq 0 ]

#!/usr/bin/env bash
# Smoke-test for execve policy mediation. Runs inside virtme-ng.
# Written to use bash builtins for the test harness itself so a fail-closed
# policy can't break the script (printf/[[/$EPOCHREALTIME are all builtins).
PROJECT=/home/ponq/dev/armadillo
KO=$PROJECT/src/module/armadillo.ko
POLICYD=$PROJECT/src/policyd/policyd.py

PASS=0
FAIL=0
pass() { printf '  PASS: %s\n' "$*"; PASS=$((PASS+1)); }
fail() { printf '  FAIL: %s\n' "$*"; FAIL=$((FAIL+1)); }
section() { printf '\n=== %s ===\n' "$*"; }

# exec_ok / exec_denied — tiny probe via bash -c, returns 0/1.
exec_ok() { bash -c 'exec true' 2>/dev/null; }

printf '%s\n' "==================================="
printf '%s\n' " Armadillo policy smoke-test"
printf '%s\n' "==================================="

section "Load module"
insmod "$KO" || { fail "insmod failed"; exit 1; }
pass "module loaded"

section "Default config (fail-open, no daemon)"
if exec_ok; then
    pass "exec allowed with no daemon (fail-open default)"
else
    fail "exec blocked without daemon"
fi

section "Attach daemon — execs mediated, verdict=ALLOW"
"$POLICYD" --timeout-ms 2000 > /tmp/policyd.log 2>&1 &
DAEMON_PID=$!
sleep 0.3
if ! kill -0 $DAEMON_PID 2>/dev/null; then
    fail "policyd died immediately"
    cat /tmp/policyd.log
    rmmod armadillo
    exit 1
fi
pass "policyd running (pid=$DAEMON_PID)"

bash -c 'exec /bin/ls /dev/null >/dev/null'
sleep 0.2
if grep -q 'path=/bin/ls' /tmp/policyd.log; then
    pass "daemon saw the ls exec and replied ALLOW"
else
    fail "daemon didn't see ls exec"
    printf -- '--- policyd.log ---\n'
    cat /tmp/policyd.log
fi

section "Timeout test — SIGSTOP'd daemon, short timeout, fail-closed"
# Reconfigure the already-attached daemon via a second short-lived attach:
# we can't, only one daemon at a time. Instead, kill current daemon and
# start a new one with --fail-closed --timeout-ms 300.
kill $DAEMON_PID
wait $DAEMON_PID 2>/dev/null
"$POLICYD" --fail-closed --timeout-ms 300 > /tmp/policyd.log 2>&1 &
DAEMON_PID=$!
sleep 0.3
kill -STOP $DAEMON_PID

T0=$EPOCHREALTIME
if bash -c 'exec /bin/true' 2>/dev/null; then
    fail "exec should have been denied on timeout"
else
    T1=$EPOCHREALTIME
    # $EPOCHREALTIME is "sec.usec" — compute elapsed ms with bash arithmetic
    T0_MS=$(( ${T0/./} / 1000 ))
    T1_MS=$(( ${T1/./} / 1000 ))
    ELAPSED_MS=$(( T1_MS - T0_MS ))
    if [[ $ELAPSED_MS -ge 200 && $ELAPSED_MS -le 1500 ]]; then
        pass "exec denied after timeout (~${ELAPSED_MS}ms)"
    else
        fail "timeout unexpected duration: ${ELAPSED_MS}ms"
    fi
fi

kill -CONT $DAEMON_PID
kill $DAEMON_PID
wait $DAEMON_PID 2>/dev/null
sleep 0.2

section "Daemon graceful exit restores fail-open"
if exec_ok; then
    pass "exec allowed after daemon graceful shutdown"
else
    fail "exec still blocked — fail-open not restored on exit"
fi

section "rmmod cleanly with no daemon attached"
rmmod armadillo
# lsmod is an execve — check via sysfs which is a builtin-friendly path
if [[ -e /sys/module/armadillo ]]; then
    fail "module still loaded after rmmod"
else
    pass "module unloaded cleanly"
fi

section "Kernel log — last armadillo lines"
dmesg | grep -i armadillo | tail -14

printf '\n===================================\n'
printf ' Results: %d passed, %d failed\n' $PASS $FAIL
printf '===================================\n'
[[ $FAIL -eq 0 ]]

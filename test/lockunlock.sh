use_sudo=sudo

$use_sudo rmmod armadillo

$use_sudo insmod ../src/armadillo.ko
RET=$?
if [ $RET -ne 0 ]
then
    echo "Unable to insmod the module. return: $RET"
    exit $RET
fi
echo "Module successfully insmoded"
echo "lsmod:"
lsmod |grep armadillo

$use_sudo ../src/interface lock lockpassword
RET=$?
if [ $RET -ne 0 ]
then
    echo "Unable to lock the module. return: $RET"
    exit $RET
fi
echo "Module successfully locked"

$use_sudo rmmod armadillo.ko
RET=$?
if [ $RET -eq 0 ]
then
    echo "We were able to unload the module, failed: $RET"
    exit 1
fi
echo "Unable to unload module - success"

$use_sudo ../src/interface lock lockpassword
RET=$?
if [ $RET -eq 0 ]
then
    echo "Shouldnt be possible to lock an already locked module, failed: $RET"
    exit 1
fi
echo "Unable to lock already locked module - success"

$use_sudo ../src/interface unlock lock
RET=$?
if [ $RET -eq 0 ]
then
    echo "Shouldnt be possible to unlock with wrong password, failed: $RET"
    exit 1
fi
echo "Unable to unlock with wrong password - success"

$use_sudo ../src/interface unlock lockpassword
RET=$?
if [ $RET -ne 0 ]
then
    echo "Should be possible to unlock with correct password, failed: $RET"
    exit 1
fi
echo "Unlocked with correct password - success"

$use_sudo rmmod armadillo.ko
RET=$?
if [ $RET -ne 0 ]
then
    echo "Unable to rmmod the module. return: $RET"
    exit $RET
fi
echo "Module successfully rmmod'ded"
echo "lsmod:"
lsmod |grep armadillo

exit 0

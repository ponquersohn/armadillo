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

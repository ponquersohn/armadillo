# Armadillo - a custom made TPE solution
Created for the need of LS 2020. This is a custom hardening tool that will be able to deescalate root priviledges and prevent breaking the machine

Version 0.0000002 - two evenings of coding:
# Included: 
- loadable module with userspace interface 
- feature - protecting a PID from killing - set a selected PID unkillable by SIGKILL and SIGTERM!!!!. Still one can run sigsegv and others but its easy to catch by user appliaction.
- feature - added the ability to lock and unlock configuration changes with password
- feature - protecting the module from being unloaded (with --force too) when locked
- feature - implemented syscall hooking with ftrace
- feature - password protect locked state with AES in mem encrypton and random key.

- syscall hook - hooked ioctl to control the behavior of chattr

# Install
Prereq:
yum install kernel-devel-$(uname -r) -y
```
Centos8 # make
cd src && make all
make[1]: Entering directory '/root/compile/armadillo/src'
cd module && make
make[2]: Entering directory '/root/compile/armadillo/src/module'
make -C /lib/modules/4.18.0-240.15.1.el8_3.x86_64+debug/build M=/root/compile/armadillo/src/module -I../  modules
make[3]: Entering directory '/mnt/rpm/src/kernels/4.18.0-240.15.1.el8_3.x86_64+debug'
  CC [M]  /root/compile/armadillo/src/module/module.o
  CC [M]  /root/compile/armadillo/src/module/ftrace_hooker.o
  CC [M]  /root/compile/armadillo/src/module/obfuscate.o
  CC [M]  /root/compile/armadillo/src/module/command_ioctl.o
  LD [M]  /root/compile/armadillo/src/module/armadillo.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /root/compile/armadillo/src/module/armadillo.mod.o
  LD [M]  /root/compile/armadillo/src/module/armadillo.ko
make[3]: Leaving directory '/mnt/rpm/src/kernels/4.18.0-240.15.1.el8_3.x86_64+debug'
make[2]: Leaving directory '/root/compile/armadillo/src/module'
cp -f module/armadillo.ko .
cd userspace && make
make[2]: Entering directory '/root/compile/armadillo/src/userspace'
rm -f  interface.o
rm -f interface.static interface
gcc  -c interface.c   -I../
gcc  -o interface interface.o -I../
gcc  --static -o interface.static interface.o -I../
make[2]: Leaving directory '/root/compile/armadillo/src/userspace'
cp -f userspace/interface .
chmod +x interface
make[1]: Leaving directory '/root/compile/armadillo/src'
```

# Examples

## Loading and unloading
```
Centos8 # insmod armadillo.ko 
Centos8 # tail -5 /var/log/messages|grep armadillo
Mar 29 13:05:11 ip-10-0-0-36 kernel: armadillo: original ioctl() ret: -82890694263136
Mar 29 13:05:11 ip-10-0-0-36 kernel: armadillo: hooked ioctl() fd: -2111242592, req: 1036500352l path: /dev/ptmx
Mar 29 13:05:11 ip-10-0-0-36 kernel: armadillo: original ioctl() ret: -82890685088096
Mar 29 13:05:11 ip-10-0-0-36 kernel: armadillo: hooked ioctl() fd: -2111242592, req: 1036500352l path: /dev/ptmx
Mar 29 13:05:11 ip-10-0-0-36 kernel: armadillo: original ioctl() ret: -82890685088096
Centos8 # rmmod armadillo.ko 
```
## Locking config and preventing module unload
```
Centos8 # insmod armadillo.ko 
Centos8 # ./userspace/interface lock password
Centos8 # rmmod armadillo.ko 
rmmod: ERROR: Module armadillo is in use
Centos8 # rmmod --force armadillo.ko 
rmmod: ERROR: could not remove 'armadillo': Resource temporarily unavailable
rmmod: ERROR: could not remove module armadillo.ko: Resource temporarily unavailable
Centos8 # ./userspace/interface unlock password
Centos8 # rmmod armadillo.ko 
```

## Making a process unkillable

```
[dev_machine] /root/compile/armadillo/src # insmod armadillo.ko 
[dev_machine] /root/compile/armadillo/src # sleep 3600 & PID=$!
[1] 24719
[dev_machine] /root/compile/armadillo/src # ./interface set_unkillable $PID on
pid: 24719, new_status: 1
[dev_machine] /root/compile/armadillo/src # kill $PID
```
And with disabled protection:
```
[dev_machine] /root/compile/armadillo/src # ./interface set_unkillable $PID off
pid: 24719, new_status: 0
[dev_machine] /root/compile/armadillo/src # kill $PID
[1]+  Terminated              sleep 3600
```

# Preventing module unload
```
[root@ip-10-0-0-36 armadillo]# insmod armadillo.ko 
[root@ip-10-0-0-36 armadillo]# ./userspace/interface lock password
[root@ip-10-0-0-36 armadillo]# rmmod armadillo.ko 
rmmod: ERROR: Module armadillo is in use
[root@ip-10-0-0-36 armadillo]# rmmod --force armadillo.ko 
rmmod: ERROR: could not remove 'armadillo': Resource temporarily unavailable
rmmod: ERROR: could not remove module armadillo.ko: Resource temporarily unavailable
```
And after disabling the protection
```
[root@ip-10-0-0-36 armadillo]# ./userspace/interface unlock password
[root@ip-10-0-0-36 armadillo]# rmmod armadillo.ko 
```

# TODO
## MUSt have:
  - disable remount with rw for root
  - disable rm -rf /
  - add rm protection for specified files
  - disable some system_calls for root - 
  - delayed arming - armadillo will arm itself after specified period of time - for example to wait until boot finishes

## To think about:
	- obfuscate the module by encrypting the code and decrypting in memory
	- do a terminal interface via sockets - this is heavy lift but doable... if its needed.

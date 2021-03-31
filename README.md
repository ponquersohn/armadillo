# Armadillo - a custom made TPE solution
Created for the need of LS 2020. This is a custom hardening tool that will be able to deescalate root priviledges and prevent breaking the machine

Version 0.0000001 - one evening of coding:
# Included: 
- loadable module with userspace interface 
- feature - protecting a PID from killing - set a selected PID unkillable by SIGKILL and SIGTERM!!!!. Still one can run sigsegv and others but its easy to catch by user appliaction.
- feature - added the ability to lock and unlock configuration changes with password
- feature - protecting the module from being unloaded (with --force too) when locked
- feature - implemented syscall hooking with ftrace
- syscall hook - hooked ioctl to control the behavior of chattr

# Install
Prereq:
yum install kernel-devel-$(uname -r) -y
```
Centos8 # make all
cd module && make
make[1]: Entering directory '/root/compile/armadillo/module'
make[1]: Nothing to be done for 'all'.
make[1]: Leaving directory '/root/compile/armadillo/module'
cp -f module/armadillo.ko .
cd userspace && make
make[1]: Entering directory '/root/compile/armadillo/userspace'
rm -f  interface.o
rm -f interface.static interface
gcc  -c interface.c  
gcc  -o interface interface.o
gcc  --static -o interface.static interface.o
make[1]: Leaving directory '/root/compile/armadillo/userspace'
cp -f userspace/interface .
chmod +x interface
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


root@kali:~/ls2020/homebrew_tools/armadillo#
root@kali:~/ls2020/homebrew_tools/armadillo# ls
armadillo.c  interface.c  ioctl.h  Makefile  readme.md
root@kali:~/ls2020/homebrew_tools/armadillo# make
make -C /lib/modules/5.3.0-kali2-amd64/build M=/root/ls2020/homebrew_tools/armadillo modules
make[1]: Entering directory '/usr/src/linux-headers-5.3.0-kali2-amd64'
  CC [M]  /root/ls2020/homebrew_tools/armadillo/armadillo.o
/root/ls2020/homebrew_tools/armadillo/armadillo.c: In function ‘init_module’:
/root/ls2020/homebrew_tools/armadillo/armadillo.c:97:5: warning: ISO C90 forbids mixed declarations and code [-Wdeclaration-after-statement]
   97 |     int ret;
      |     ^~~
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /root/ls2020/homebrew_tools/armadillo/armadillo.mod.o
  LD [M]  /root/ls2020/homebrew_tools/armadillo/armadillo.ko
make[1]: Leaving directory '/usr/src/linux-headers-5.3.0-kali2-amd64'
gcc -o interface interface.c
root@kali:~/ls2020/homebrew_tools/armadillo# ls -la
total 880
drwxr-xr-x 2 root root   4096 Mar  1 02:25 .
drwxr-xr-x 3 root root   4096 Mar  1 02:06 ..
-rw-r--r-- 1 root root   4102 Mar  1 01:56 armadillo.c
-rw-r--r-- 1 root root 352544 Mar  1 02:25 armadillo.ko
-rw-r--r-- 1 root root    354 Mar  1 02:25 .armadillo.ko.cmd
-rw-r--r-- 1 root root     51 Mar  1 02:25 armadillo.mod
-rw-r--r-- 1 root root   1162 Mar  1 02:25 armadillo.mod.c
-rw-r--r-- 1 root root    180 Mar  1 02:25 .armadillo.mod.cmd
-rw-r--r-- 1 root root 141808 Mar  1 02:25 armadillo.mod.o
-rw-r--r-- 1 root root  50906 Mar  1 02:25 .armadillo.mod.o.cmd
-rw-r--r-- 1 root root 212248 Mar  1 02:25 armadillo.o
-rw-r--r-- 1 root root  58020 Mar  1 02:25 .armadillo.o.cmd
-rwxr-xr-x 1 root root  17080 Mar  1 02:25 interface
-rw-r--r-- 1 root root   2368 Mar  1 01:42 interface.c
-rw-r--r-- 1 root root    935 Mar  1 01:32 ioctl.h
-rw-r--r-- 1 root root    190 Mar  1 02:20 Makefile
-rw-r--r-- 1 root root     51 Mar  1 02:25 modules.order
-rw-r--r-- 1 root root      0 Mar  1 02:25 Module.symvers
-rw-r--r-- 1 root root    385 Mar  1 02:21 readme.md
root@kali:~/ls2020/homebrew_tools/armadillo# sleep 3600 &
[1] 25881
root@kali:~/ls2020/homebrew_tools/armadillo# ps
    PID TTY          TIME CMD
   1805 pts/1    00:00:00 bash
  25881 pts/1    00:00:00 sleep
  25882 pts/1    00:00:00 ps
root@kali:~/ls2020/homebrew_tools/armadillo# kill 25881
root@kali:~/ls2020/homebrew_tools/armadillo#
[1]+  Terminated              sleep 3600
root@kali:~/ls2020/homebrew_tools/armadillo# sleep 3600 &
[1] 25883
root@kali:~/ls2020/homebrew_tools/armadillo#
root@kali:~/ls2020/homebrew_tools/armadillo# ./interface toggle_unkillable 25883 on
Can't open device file: /dev/armadillo_cdrv
root@kali:~/ls2020/homebrew_tools/armadillo# insmod armadillo.ko
root@kali:~/ls2020/homebrew_tools/armadillo# ./interface toggle_unkillable 25883 on
pid: 25883, new_status: 1
root@kali:~/ls2020/homebrew_tools/armadillo# kill 25883
root@kali:~/ls2020/homebrew_tools/armadillo# ps
    PID TTY          TIME CMD
   1805 pts/1    00:00:00 bash
  25883 pts/1    00:00:00 sleep
  25898 pts/1    00:00:00 ps
root@kali:~/ls2020/homebrew_tools/armadillo# kill -9 25883
root@kali:~/ls2020/homebrew_tools/armadillo# ps
    PID TTY          TIME CMD
   1805 pts/1    00:00:00 bash
  25883 pts/1    00:00:00 sleep
  25899 pts/1    00:00:00 ps
root@kali:~/ls2020/homebrew_tools/armadillo# ./interface toggle_unkillable 25883 off
pid: 25883, new_status: 0
root@kali:~/ls2020/homebrew_tools/armadillo# kill 25883
root@kali:~/ls2020/homebrew_tools/armadillo#
[1]+  Terminated              sleep 3600
root@kali:~/ls2020/homebrew_tools/armadillo#
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


More will come here:
- To do: - control root actions:
	- disable remount with rw for root
	- disable rm -rf /
	- disable module unloading
	- add rm protection for specified files
	- disable some system_calls for root - 
	- delayed arming - armadillo will arm itself after specified period of time - for example to wait until boot finishes
- To think about: 
	- obfuscate the module by encrypting the code and decrypting in memory
	- do a terminal interface via sockets - this is heavy lift but doable... if its needed.

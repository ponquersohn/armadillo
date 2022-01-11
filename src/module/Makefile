MODULE_NAME := armadillo
#CFLAGS=-I../
# OBFUSCATION_KEY := $(shell cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 40 | head -n 1)

OBFUSCATION_KEY := $(shell python -c 'import os;print(", ".join([ "0x%02x" % b for b in os.urandom(32)]))')
$(info $(OBFUSCATION_KEY))
MODULE_SRC=../
EXTRA_CFLAGS += -I$(MODULE_SRC)

KBUILD_CFLAGS_MODULE += -Werror -DOBFUSCATION_KEY

ifeq ($(KERNELRELEASE),)
# 'Out-of-kernel' part

MODULE_SOURCES := \
	module.c \
	ftrace_hooker.c \
	obfuscate.c \
	command_ioctl.c

UNAME := $(shell uname -r)
KBUILD_DIR := /lib/modules/$(UNAME)/build
PWD := $(shell pwd)

all: $(MODULE_NAME).ko

$(MODULE_NAME).ko: $(MODULE_SOURCES)
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) $(EXTRA_CFLAGS)  modules

clean:
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) $(EXTRA_CFLAGS) clean
	
else
# KBuild part. 
# It is used by the kernel build system to actually build the module.
# ccflags-y :=  -I$(src)

obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-y := \
	module.o \
	ftrace_hooker.o \
	obfuscate.o \
	command_ioctl.o 

endif

.PHONY: clean
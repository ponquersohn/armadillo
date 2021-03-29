MODULE_NAME := armadillo

KBUILD_CFLAGS_MODULE += -Werror

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
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) clean
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

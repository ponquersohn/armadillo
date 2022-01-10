module:
	cd module && $(MAKE)
	cp -f module/armadillo.ko .

userspace:
	cd userspace && $(MAKE)
	cp -f userspace/interface .
	chmod +x interface

clean_module:
	cd module && $(MAKE) clean

clean_userspace: 
	cd userspace && $(MAKE) clean

all: module userspace

test: all
	cd ../test && $(MAKE)

clean: clean_module clean_userspace
	rm -f interface armadillo.ko

.PHONY: clean all module userspace clean_module clean_userspace

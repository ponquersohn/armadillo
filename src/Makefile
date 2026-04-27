module:
	cd module && $(MAKE)
	cp -f module/armadillo.ko .

userspace:
	cd userspace && $(MAKE)
	cp -f userspace/interface .
	chmod +x interface

policyd:
	cd policyd && $(MAKE)

clean_module:
	cd module && $(MAKE) clean

clean_userspace:
	cd userspace && $(MAKE) clean

clean_policyd:
	cd policyd && $(MAKE) clean

all: module userspace policyd

test: all
	cd ../test && $(MAKE)

clean: clean_module clean_userspace clean_policyd
	rm -f interface armadillo.ko

.PHONY: clean all module userspace policyd clean_module clean_userspace clean_policyd

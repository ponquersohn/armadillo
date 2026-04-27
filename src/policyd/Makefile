PYTHON ?= python3
SOURCES := policyd.py ioctl_defs.py

.PHONY: all clean

all:
	$(PYTHON) -m py_compile $(SOURCES)

clean:
	rm -rf __pycache__

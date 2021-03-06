CC := gcc
CLAGS := 
INTERFACE_NAME := interface

# EXTRA_CFLAGS += -I$(src)

.PHONY: clean all 
.DEFAULT_GOAL := $(INTERFACE_NAME)

UNAME := $(shell uname -r)
PWD := $(shell pwd)

SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES) $(LIB_PATH) $(LIBS)

$(INTERFACE_NAME): $(INTERFACE_NAME).o
	$(CC) $(CFLAGS) -o $(INTERFACE_NAME) $(INTERFACE_NAME).o
    
all: clean $(INTERFACE_NAME)

clean:
	rm -f $(OBJECTS)


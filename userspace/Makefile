CC := gcc
CLAGS := 
INTERFACE_NAME := interface
INTERFACE_NAME_STATIC := $(INTERFACE_NAME).static

# EXTRA_CFLAGS += -I$(src)


PWD := $(shell pwd)

SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c, %.o, $(SOURCES))

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES) $(LIB_PATH) $(LIBS)

$(INTERFACE_NAME): $(INTERFACE_NAME).o
	$(CC) $(CFLAGS) -o $(INTERFACE_NAME) $(INTERFACE_NAME).o

$(INTERFACE_NAME_STATIC): $(INTERFACE_NAME).o
	$(CC) $(CFLAGS) --static -o $(INTERFACE_NAME_STATIC) $(INTERFACE_NAME).o

all: clean $(INTERFACE_NAME) $(INTERFACE_NAME_STATIC)

clean:
	rm -f $(OBJECTS)
	rm -f $(INTERFACE_NAME_STATIC) $(INTERFACE_NAME)

.PHONY: clean all 
.DEFAULT_GOAL := all

UNAME := $(shell uname -r)
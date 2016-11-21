CFLAGS := -Wall -fPIC
INCLUDES := -I../include
LDFLAGS :=
LIBS :=

CROSS_COMPILE ?= arm-linux-gnueabi-
CC := $(CROSS_COMPILE)gcc

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

NAME := nxsv4l2testcommon
LIB_TARGET := lib$(NAME).so

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $^

$(LIB_TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(LIB_TARGET) -o $@ $^ $(LIBS)

install: $(LIB_TARGET)
	cp $^ ../libs
	cp nxs-v4l2-test-common.h ../include

all: $(LIB_TARGET)

.PHONY: clean

clean:
	rm -f *.o
	rm -f $(LIB_TARGET)

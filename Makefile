# Makefile for ParkinDSF Redis module.

SRCS = module.c
OBJS = $(SRCS:.c=.o)
CC = gcc
CCFLAGS = -g -fPIC -Wall -I../redis/src
LD = ld
LDFLAGS = -shared -Bsymbolic
LIB = parkindsf.so

all: $(LIB)

$(LIB): $(OBJS)
	$(LD) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CCFLAGS)

clean:
	rm -rf $(OBJS) $(LIB)

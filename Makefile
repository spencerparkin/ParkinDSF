# Makefile for ParkinDSF Redis module.

SRCS = module.c dsf.c dsf_commands.c
OBJS = $(SRCS:.c=.o)
CC = gcc
CCFLAGS = -g -fPIC -std=gnu99 -Wall -I../redis/src
LD = ld
LDFLAGS = -shared -Bsymbolic -lc
LIB = parkindsf.so

all: $(LIB)

$(LIB): $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CCFLAGS)

clean:
	rm -rf $(OBJS) $(LIB)

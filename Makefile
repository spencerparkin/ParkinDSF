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

dep:
	makedepend -- $(CCFLAGS) -- $(SRCS)
	rm Makefile.bak

# DO NOT DELETE

module.o: module.h ../redis/src/redismodule.h /usr/include/stdint.h
module.o: /usr/include/stdio.h dsf.h dsf_commands.h
dsf.o: dsf.h ../redis/src/redismodule.h /usr/include/stdint.h
dsf.o: /usr/include/stdio.h /usr/include/stdlib.h
dsf_commands.o: dsf_commands.h ../redis/src/redismodule.h
dsf_commands.o: /usr/include/stdint.h /usr/include/stdio.h dsf.h

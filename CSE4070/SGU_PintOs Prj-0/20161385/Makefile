CC=gcc
TARGET=testlib
CFLAGS=-g -Wall -o
OBJS=testlib.c commands.h commands.c list.h list.c hash.h hash.c bitmap.h bitmap.c debug.h debug.c hex_dump.h hex_dump.c round.h limits.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $@ $^
clean:
	rm -rf *.o
	rm -rf $(TARGET)


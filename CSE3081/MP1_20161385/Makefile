CC=g++
CFLAGS=-g -Wall -o
OBJS=mp1_20161385.o
TARGET=mp1_20161385

all: $(TARGET)

clean:
	rm -rf *.o
	rm -rf $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $@ $(OBJS)

mp1_20161385.o: mp1_20161385.cpp

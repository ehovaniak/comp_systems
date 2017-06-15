TARGET = process
LIBS = -lm -lpthread
CC = gcc
CFLAGS = -g -Wall -Wconversion

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@
	
.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

test:
	./$(TARGET) test1.bin test1.txt
	./$(TARGET) test2.bin test2.txt
	./$(TARGET) test3.bin test3.txt
	
clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f *.txt

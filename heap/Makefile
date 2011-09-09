CC = gcc
CFLAGS = -g

RM = rm -f

HDR =
HDR += heap.h
HDR += util.h

OBJ =
OBJ += heap.o
OBJ += util.o

huffman: huffman.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

heap.o: util.h

.PHONY: clean

clean:
	$(RM) *.o huffman

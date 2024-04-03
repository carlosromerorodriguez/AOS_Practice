OBJS    = main.o common/cat.o common/info.o common/tree.o ext2/ext2_reader.o fat16/fat16_reader.o
SOURCE  = main.c common/cat.c common/info.c common/tree.c ext2/ext2_reader.c fat16/fat16_reader.c
HEADER  = common/cat.h common/info.h common/tree.h ext2/ext2_reader.h fat16/fat16_reader.h
OUT     = ../fsutils
CC      = gcc
FLAGS   = -g -c -Wall -Wextra
LFLAGS  =

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)
	rm -f $(OBJS)

main.o: main.c $(HEADER)
	$(CC) $(FLAGS) main.c -o main.o

common/%.o: common/%.c common/%.h
	$(CC) $(FLAGS) $< -o $@

ext2/%.o: ext2/%.c ext2/%.h
	$(CC) $(FLAGS) $< -o $@

fat16/%.o: fat16/%.c fat16/%.h
	$(CC) $(FLAGS) $< -o $@

clean:
	rm -f $(OBJS) $(OUT)

.PHONY: clean all
CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -pedantic -O3 -fno-strict-aliasing -ggdb
SRCS=bstree.c main.c
HDRS=bstree.h
OBJS=bstree.o main.o

main.out: $(HDRS) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o main.out

main.o: $(HDRS) main.c

bstree.o: bstree.c bstree.h

tags: $(HDRS) $(SRCS)
	ctags -R .

clean:
	rm -f *.out *.o

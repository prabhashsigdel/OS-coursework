CC = gcc
CFLAGS = -Wall -Wextra

OBJS = main.o fifo.o lru.o

paging: $(OBJS)
	$(CC) $(CFLAGS) -o paging $(OBJS)

%.o: %.c sim.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f paging *.o

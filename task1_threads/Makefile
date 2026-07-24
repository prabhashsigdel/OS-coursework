CC = gcc
CFLAGS = -Wall -Wextra -pthread

OBJS = main.o input.o process_demo.o race_demo.o prodcons.o deadlock_demo.o scheduler.o

task1: $(OBJS)
	$(CC) $(CFLAGS) -o task1 $(OBJS)

%.o: %.c demos.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f task1 *.o

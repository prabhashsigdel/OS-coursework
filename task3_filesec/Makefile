CC = gcc
CFLAGS = -Wall -Wextra

OBJS = main.o auth.o fileops.o crypto.o audit.o

task3: $(OBJS)
	$(CC) $(CFLAGS) -o task3 $(OBJS)

%.o: %.c fsec.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f task3 *.o

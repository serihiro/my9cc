PROGRAM_NAME = 9cc
CC = gcc
CFLAGS = -g3 -O1 -Wall -Wextra
OBJS = 9cc.o

main: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM_NAME) $(OBJS)

9cc.o: 9cc.c
	$(CC) $(CFLAGS) -c 9cc.c

test:
	./test.sh

clean:
	rm -rf ./*.o ./*.s ./9cc ./tmp


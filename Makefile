PROGRAM_NAME = 9cc
CC = gcc
CFLAGS = -g3 -O1 -Wall -Wextra -std=c11
OBJS = 9cc.o

main: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROGRAM_NAME) $(OBJS)

9cc.o: 9cc.c
	$(CC) $(CFLAGS) -c 9cc.c

test:
	./9cc -test
	./test.sh

format:
	clang-format 9cc.c -i

clean:
	rm -rf ./*.o ./*.s ./9cc ./tmp


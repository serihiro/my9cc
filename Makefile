PROGRAM_NAME = 9cc
CC = gcc
CFLAGS = -g3 -O1 -Wall -Wextra -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

main: $(OBJS)
	$(CC) -o $(PROGRAM_NAME) $(OBJS) $(LDFLAGS) $(CFLAGS)

$(OBJS): 9cc.h

test: 
	@make clean
	@make main
	./9cc -test
	./test.sh

format:
	clang-format $(SRCS) -i

clean:
	rm -rf ./*.o ./*.s ./9cc ./tmp


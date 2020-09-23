#Makefile for mmake.c: a program recreating a simple version of UNIX's make command

CC=gcc
FLAGS1= -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic
FLAGS2= -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

mmake: mmake.c parser.c
	$(CC) $(FLAGS1) $(FLAGS2) mmake.c -o mmake

#add clean commmand
clean:
	rm -f *.o core
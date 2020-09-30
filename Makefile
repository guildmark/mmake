#Makefile for mmake.c: a program recreating a simple version of UNIX's make command

CC=gcc
FLAGS1= -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic
FLAGS2= -Wold-style-definition

mmake: mmake.c parser.c
	$(CC) $(FLAGS1) $(FLAGS2) mmake.c parser.c -o mmake

#add clean commmand
clean:
	rm -f *.o core

#
#   module  : makefile
#   version : 1.1
#   date    : 07/22/22
#
CC = gcc
CFLAGS = -O3 -Wall -Wextra -Wno-unused-parameter -Werror

all: 32syrecc 32syreci dump

32syrecc: 32syrecc.o
	$(CC) -o$@ 32syrecc.o

32syreci: 32syreci.o
	$(CC) -o$@ 32syreci.o

dump: dump.o
	$(CC) -o$@ dump.o

clean:
	rm -f *.o

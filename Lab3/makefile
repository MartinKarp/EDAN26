CC = gcc
CFLAGS = -pthread
all:
	$(CC) swish.c $(CFLAGS) -o swish_par
	$(CC) swish_org.c -o swish_org
run:
	./swish_org
	./swish_par

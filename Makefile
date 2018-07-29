# +---------------------------------------------+
# | FILE   : Makefile                           |
# | AUTHOR : Jeffrey Hunter                     |
# | WEB    : http://www.iDevelopment.info       |
# | DATE   : 27-AUG-2002                        |
# | NOTE   : Makefile for Queue data structure. |
# +---------------------------------------------+

CC=gcc  -O3
all: cftp-serv cftp-recv
cftp-serv: server.o readfile.o queue.o minilzo.o
	$(CC) -lpthread server.o readfile.o queue.o minilzo.o -o $@

testRead: testRead.o readfile.o readfile.h minilzo.o queue.h
	$(CC) testRead.o readfile.o minilzo.o -o $@

testQueue: testQueue.o queue.o queue.h
	$(CC) -lpthread testQueue.o queue.o -o $@

cftp-recv: recv.o minilzo.o
	$(CC) -lpthread recv.o minilzo.o -o $@
clean:
	rm -f a.out core *.o testQueue recv server cftp-serv cftp-recv


CC=gcc
CFLAGS=-I/usr/include/mysql 

optimize : list.o performance.o utilities.o db.o
	$(CC) -o optimize list.o performance.o db.o utilities.o -lm -lmysqlclient

list.o : src/list.c src/list.h 
	$(CC) -c src/list.c

db.o : src/db.c src/db.h 
	$(CC) $(CFLAGS) -c src/db.c

performance.o : src/performance.c 
	$(CC) $(CFLAGS) -c src/performance.c

utilities.o : src/utilities.c src/utilities.h
	$(CC) -c src/utilities.c

clean :
	rm optimize db.o performance.o list.o utilities.o

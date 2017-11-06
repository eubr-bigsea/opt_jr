CC=gcc
CFLAGS=-I/usr/include/mysql -g

OPT_JR : boundsCalculation.o db.o initialize.o interpolation.o list.o localSearch.o main.o objectiveFunction.o openMP.o predictor.o u_debug.o u_file.o u_misc.o u_string.o
	$(CC) -fopenmp -o OPT_JR boundsCalculation.o db.o initialize.o interpolation.o list.o localSearch.o main.o objectiveFunction.o openMP.o predictor.o u_debug.o u_file.o u_misc.o u_string.o -lm -lmysqlclient

localSearch.o : src/localSearch.c src/localSearch.h
	$(CC) $(CFLAGS) -fopenmp -c src/localSearch.c

list.o : src/list.c src/list.h
	$(CC) $(CFLAGS) -c src/list.c

db.o : src/db.c src/db.h
	$(CC) $(CFLAGS) -c src/db.c

main.o : src/main.c src/main.h
	$(CC) $(CFLAGS) -c src/main.c

u_string.o : src/u_string.c src/u_string.h
	$(CC) $(CFLAGS) -c src/u_string.c

u_file.o : src/u_file.c src/u_file.h
	$(CC) $(CFLAGS) -c src/u_file.c

u_debug.o : src/u_debug.c src/u_debug.h
	$(CC) $(CFLAGS) -c src/u_debug.c

u_misc.o : src/u_misc.c src/u_misc.h
	$(CC) $(CFLAGS) -c src/u_misc.c

initialize.o : src/initialize.c src/initialize.h
	$(CC) $(CFLAGS) -c src/initialize.c

interpolation.o : src/interpolation.c src/interpolation.h
	$(CC) $(CFLAGS) -c src/interpolation.c

objectiveFunction.o : src/objectiveFunction.c src/objectiveFunction.h
	$(CC) $(CFLAGS) -c src/objectiveFunction.c

openMP.o : src/objectiveFunction.c src/openMP.h
	$(CC) $(CFLAGS) -c src/openMP.c

predictor.o : src/predictor.c src/predictor.h
	$(CC) $(CFLAGS) -c src/predictor.c

boundsCalculation.o : src/boundsCalculation.c src/boundsCalculation.h
	$(CC) $(CFLAGS) -c src/boundsCalculation.c

clean :
	rm OPT_JR boundsCalculation.o db.o initialize.o interpolation.o list.o localSearch.o main.o objectiveFunction.o openMP.o predictor.o u_debug.o u_file.o u_misc.o u_string.o

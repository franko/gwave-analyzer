CC = gcc
OFL = -O2 -ffast-math -fomit-frame-pointer
DFL = -g
CFLAGS = $(OFL) -Wall
LIB = -lm

OBJ = comune.o main.o wtm.o # gtkanalisi.o fourier.o

all: gwave

gwave: $(OBJ)
	$(CC) -o gwave $(OBJ) $(LIB)

main.o: main.c comune.h
	$(CC) $(CFLAGS) -c main.c

comune.o: comune.c comune.h
	$(CC) $(CFLAGS) -c comune.c

# fourier.o: fourier.c comune.h gtkcomune.h
# 	$(CC) $(CFLAGS) $(XINC) $(GTKINC) -c fourier.c

wtm.o: wtm.c comune.h
	$(CC) $(CFLAGS) -c wtm.c

clean:
	rm *.o

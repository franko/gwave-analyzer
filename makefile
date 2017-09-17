CC = gcc
OFL= -O3 -fstrength-reduce -fmove-all-movables -frerun-loop-opt -funroll-loops -ffast-math -fomit-frame-pointer -Wno-uninitialized
#OFL= -O3 -fstrength-reduce -fmove-all-movables -frerun-loop-opt -funroll-loops -ffast-math -Wno-uninitialized
DFL= -g
#PROF= -pg
PROF=
CFLAGS = -pipe $(OFL) $(PROF) -Wall
LIB =  -lm
#PLLIB= -L/usr/X11R6/lib -lplot -lXaw -lXmu -lXt -lSM -lICE -lXext -lX11
XINC= -I/usr/X11R6/include
GTKINC= `gtk-config --cflags`
GTKLIB= `gtk-config --libs`

GTKDBGL= $(GTKCMP)/gtk/.static_libs/libgtk.a $(GTKCMP)/gdk/.static_libs/libgdk.a -lglib -lgmodule

OBJ= comune.o gtkanalisi.o fourier.o wtm.o

all: gwave

gwave: $(OBJ)
	$(CC) -pipe -o gwave $(OBJ) $(LIB) $(GTKLIB) $(PROF)

gtkanalisi.o: gtkanalisi.c gtkcomune.h comune.h icona.xpm
	$(CC) $(CFLAGS) $(XINC) $(GTKINC) -c gtkanalisi.c

comune.o: comune.c comune.h
	$(CC) $(CFLAGS) -c comune.c

fourier.o: fourier.c comune.h gtkcomune.h
	$(CC) $(CFLAGS) $(XINC) $(GTKINC) -c fourier.c

wtm.o: wtm.c comune.h
	$(CC) $(CFLAGS) $(GTKINC) -c wtm.c

wtm.s: wtm.c comune.h
	$(CC) -S $(CFLAGS) wtm.c

sintesi: sintesi.c
	gcc -o sintesi -O3 -Wall -fomit-frame-pointer -ffast-math sintesi.c -lm

clean:
	rm *.o


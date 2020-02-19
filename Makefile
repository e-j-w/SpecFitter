CFLAGS   = -o2 -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

all: jf3

jf3: jf3.c jf3.h read_data.c spectrum_drawing.c
	gcc jf3.c $(CFLAGS) -o jf3

clean:
	rm -rf *~ *.o jf3

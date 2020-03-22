GLIB_COMPILE_RESOURCES = `pkg-config --variable=glib_compile_resources gio-2.0`

resources = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=./data --generate-dependencies data/jf3.gresource.xml)

CFLAGS = -I. -I./src/lin_eq_solver -O3 -Wall -Wshadow -Wunreachable-code -std=c99

all: lin_eq_solver jf3-resources.c jf3

jf3: src/jf3.c src/jf3.h src/read_data.c src/read_config.c src/fit_data.c src/spectrum_drawing.c src/utils.c jf3-resources.c src/lin_eq_solver/lin_eq_solver.o
	gcc src/jf3.c $(CFLAGS) -lm `pkg-config --cflags --libs gtk+-3.0` -export-dynamic -o jf3 src/lin_eq_solver/lin_eq_solver.o
	rm jf3-resources.c

jf3-resources.c: data/jf3.gresource.xml data/jf3.glade $(resources)
	$(GLIB_COMPILE_RESOURCES) data/jf3.gresource.xml --target=jf3-resources.c --sourcedir=./data --generate-source

lin_eq_solver: src/lin_eq_solver/lin_eq_solver.c src/lin_eq_solver/lin_eq_solver.h
	gcc $(CFLAGS) -c -o src/lin_eq_solver/lin_eq_solver.o src/lin_eq_solver/lin_eq_solver.c

clean:
	rm -rf *~ *.o */*/*.o jf3-resources.c *# jf3

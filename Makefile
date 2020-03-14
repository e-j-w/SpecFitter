GLIB_COMPILE_RESOURCES = `pkg-config --variable=glib_compile_resources gio-2.0`

resources = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=. --generate-dependencies jf3.gresource.xml)

CFLAGS = -I./lin_eq_solver -o2 -Wall -lm `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

all: lin_eq_solver jf3-resources.c jf3

jf3: jf3.c jf3.h read_data.c read_config.c fit_data.c spectrum_drawing.c utils.c jf3-resources.c lin_eq_solver/lin_eq_solver.o
	gcc jf3.c $(CFLAGS) -o jf3 lin_eq_solver/lin_eq_solver.o

jf3-resources.c: jf3.gresource.xml jf3.glade $(resources)
	$(GLIB_COMPILE_RESOURCES) jf3.gresource.xml --target=jf3-resources.c --sourcedir=. --generate-source

lin_eq_solver: lin_eq_solver/lin_eq_solver.c lin_eq_solver/lin_eq_solver.h
	gcc -I./lin_eq_solver -o2 -c -o lin_eq_solver/lin_eq_solver.o lin_eq_solver/lin_eq_solver.c

clean:
	rm -rf *~ *.o */*.o jf3-resources.c jf3

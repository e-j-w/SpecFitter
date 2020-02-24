GLIB_COMPILE_RESOURCES = `pkg-config --variable=glib_compile_resources gio-2.0`

resources = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=. --generate-dependencies jf3.gresource.xml)

CFLAGS = -o2 -Wall -lm `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

all: jf3-resources.c jf3

jf3: jf3.c jf3.h read_data.c spectrum_drawing.c jf3-resources.c
	gcc jf3.c $(CFLAGS) -o jf3

jf3-resources.c: jf3.gresource.xml jf3.glade $(resources)
	$(GLIB_COMPILE_RESOURCES) jf3.gresource.xml --target=jf3-resources.c --sourcedir=. --generate-source

clean:
	rm -rf *~ *.o jf3-resources.c jf3

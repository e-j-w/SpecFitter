GLIB_COMPILE_RESOURCES = `pkg-config --variable=glib_compile_resources gio-2.0`

RESOURCES = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=./data --generate-dependencies data/specfitter.gresource.xml)

CFLAGS = -I. -I./src/utils -O2 -Wall -Wextra -Wshadow -Wunreachable-code -Wpointer-arith -Wcast-align -Wformat-security -Wstack-protector -Wconversion -std=c99 -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED

all: src/utils/lin_eq_solver.o src/utils/utils.o specfitter-resources.c specfitter

specfitter: src/specfitter.c src/specfitter.h src/gui.c src/read_data.c src/read_config.c src/write_data.c src/fit_data.c src/spectrum_drawing.c src/spectrum_data.c specfitter-resources.c src/utils/lin_eq_solver.o src/utils/utils.o
	gcc src/specfitter.c $(CFLAGS) -lm `pkg-config --cflags --libs gtk+-3.0` -export-dynamic -o specfitter src/utils/lin_eq_solver.o src/utils/utils.o

specfitter-resources.c: data/specfitter.gresource.xml data/specfitter.ui data/shortcuts_window.ui $(RESOURCES)
	$(GLIB_COMPILE_RESOURCES) data/specfitter.gresource.xml --target=specfitter-resources.c --sourcedir=./data --generate-source

src/utils/lin_eq_solver.o: src/utils/lin_eq_solver.c src/utils/lin_eq_solver.h
	gcc $(CFLAGS) -c -o src/utils/lin_eq_solver.o src/utils/lin_eq_solver.c

src/utils/utils.o: src/utils/utils.c src/utils/utils.h
	gcc $(CFLAGS) -c -o src/utils/utils.o src/utils/utils.c

install:
	@echo "Will install to /usr/bin."
	@echo "Run 'make uninstall' to undo installation."
	@if ! [ "$(shell id -u)" = 0 ]; then \
		echo "This must be run with administrator privileges (eg. with 'sudo')."; \
	else \
		cp specfitter /usr/bin ; \
		cp data/io.github.e_j_w.SpecFitter.svg /usr/share/icons/hicolor/scalable/apps ; \
		cp data/io.github.e_j_w.SpecFitter.desktop /usr/share/applications ; \
		cp data/specfitter-mime.xml /usr/share/mime/packages ; \
		update-mime-database /usr/share/mime ; \
		update-desktop-database /usr/share/applications ; \
		echo "Done!" ; \
	fi


uninstall:
	@echo "Will undo changes made from running 'make install'."
	@if ! [ "$(shell id -u)" = 0 ]; then \
		echo "This must be run with administrator privileges (eg. with 'sudo')."; \
	else \
		rm /usr/bin/specfitter ; \
		rm /usr/share/icons/hicolor/scalable/apps/io.github.e_j_w.SpecFitter.svg ; \
		rm /usr/share/applications/io.github.e_j_w.SpecFitter.desktop ; \
		rm /usr/share/mime/packages/specfitter-mime.xml ; \
		update-mime-database /usr/share/mime ; \
		update-desktop-database /usr/share/applications ; \
		echo "Done!" ; \
	fi

clean:
	rm -rf *~ *.o */*/*.o specfitter-resources.c *# specfitter

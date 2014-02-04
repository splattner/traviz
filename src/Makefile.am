# what flags you want to pass to the C compiler & linker
#CFLAGS =
#LDFLAGS =

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = traviz
traviz_SOURCES = main.c main.h \
				gui.c gui.h \
				datastore.c datastore.h \
				plot.h plot.c \
				linkedList.c linkedList.h \
				binaryToText.c binaryToText.h \
				include.h \
				binaryValue.c binaryValue.h \
				filter.c filter.h
				
traviz_CFLAGS = --pedantic -Wall -std=c99 -O0 -g -lm `pkg-config --cflags gtk+-x11-2.0 gmodule-2.0 plplotd gdk-2.0 gdk-pixbuf-2.0 gdk-x11-2.0 glib-2.0 gdk-pixbuf-xlib-2.0 cairo cairo-xlib gtk+-2.0`
traviz_LDFLAGS = -g `pkg-config --libs gmodule-2.0 gtk+-x11-2.0 plplotd gdk-2.0 gdk-pixbuf-2.0 gdk-x11-2.0 glib-2.0 gdk-pixbuf-xlib-2.0 cairo cairo-xlib gtk+-2.0`
				

travizdir = $(datadir)/@PACKAGE@
traviz_DATA = gui.glade \
				traviz.jpg


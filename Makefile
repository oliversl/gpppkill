# gpppkill Makefile
# 
# Use DESTDIR when installing to another prefix directory

# Redhat 6.x
CC	= g++
CFLAGS  = -Wall -g -Wno-unused -DGTK_DISABLE_COMPAT_H #-pedantic  
# no hace falta sacar la opcion -g porque uso 'install -s'
#CFLAGS  = -Wall -Wno-unused -DGTK_DISABLE_COMPAT_H
INCLUDE = `gtk-config --cflags`
LIBS    = `gtk-config --libs`
DESTDIR  = /
BINDIR  = ${DESTDIR}/usr/X11R6/bin

# Debian 2
#CC      = egcc
#CFLAGS  = -Wall -Wno-unused -DGTK_DISABLE_COMPAT_H # -pedantic  
#INCLUDE = -I/usr/include/g++ `gtk-config --cflags`
#LIBS    = -lstdc++ `gtk-config --libs`
#DESTDIR  = /
#BINDIR  = ${DESTDIR}/usr/X11R6/bin

all: gpppkill
	
# --- gpppkill -----------------------------------------------------------------
gpppkill : dialog.o warning.o messagebox.o graf.o listac.o rcfile.o rcgpppkill.o  preferences.o pppkill.o gpppkill.o main.o
	$(CC) $(CFLAGS) -o gpppkill dialog.o warning.o messagebox.o graf.o listac.o rcfile.o rcgpppkill.o preferences.o pppkill.o gpppkill.o main.o  $(INCLUDE) $(LIBS)

dialog.o : dialog.cc dialog.h
	$(CC) $(CFLAGS) -c dialog.cc $(INCLUDE)

warning.o : warning.cc warning.h
	$(CC) $(CFLAGS) -c warning.cc $(INCLUDE)

messagebox.o : messagebox.cc messagebox.h
	$(CC) $(CFLAGS) -c messagebox.cc $(INCLUDE)

graf.o : graf.cc graf.h
	$(CC) $(CFLAGS) -c graf.cc $(INCLUDE)

listac.o : listac.cc listac.h
	$(CC) $(CFLAGS) -c listac.cc $(INCLUDE)

rcfile.o : rcfile.cc rcfile.h
	$(CC) $(CFLAGS) -c rcfile.cc $(INCLUDE)

rcgpppkill.o : rcgpppkill.cc rcgpppkill.h
	$(CC) $(CFLAGS) -c rcgpppkill.cc $(INCLUDE)

pppkill.o : pppkill.cc pppkill.h
	$(CC) $(CFLAGS) -c pppkill.cc $(INCLUDE)

preferences.o : preferences.cc preferences.h
	$(CC) $(CFLAGS) -c preferences.cc $(INCLUDE)

gpppkill.o : gpppkill.cc gpppkill.h
	$(CC) $(CFLAGS) -c gpppkill.cc $(INCLUDE)

main.o : main.cc main.h
	$(CC) $(CFLAGS) -c main.cc $(INCLUDE) 

# --- install ------------------------------------------------------------------
install: gpppkill
	install -s -m 755 -o 0 -g 0 gpppkill $(BINDIR)/gpppkill

# --- uninstall ----------------------------------------------------------------
uninstall:
	rm -vf $(BINDIR)/gpppkill

# --- opcionales ---------------------------------------------------------------
gpppkill-e :  pppkill.o gpppkill.o main.o
	$(CC) $(CFLAGS) -o gpppkill pppkill.o gpppkill.o main.o  $(INCLUDE) $(LIBS) -lefence

.cc:
	$(CC) $(CFLAGS) -o $* $*.cc $(INCLUDE)

# --- clean --------------------------------------------------------------------
clean:
	rm -f *.o
	rm -f core

# --- cleanbak -----------------------------------------------------------------
cleanbak:
	rm -f *~

# --- distclean ----------------------------------------------------------------
distclean: clean cleanbak


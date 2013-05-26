CC=g++
CFLAGS=-O3  

all: watcher

nma.o:  hooks/nma.c
	gcc -c -fPIC hooks/nma.c -o nma.o    

watcher: main.cpp  nma.o
	$(CC) $(CFLAGS)   main.cpp nma.o `pkg-config opencv --cflags --libs sqlite3 --cflags --libs --cflags --libs glib-2.0` -lssl -lcrypto  -o $@



clean:
	rm -r  watcher


ifeq ($(shell uname -m),armv6l)
 $(info Rpi)
 CC=g++
 CFLAGS=-O3  -mfpu=vfp -mfloat-abi=hard
else
 $(info native build)
 CC=g++
 CFLAGS=-O0 -g  
 #-pedantic 
 #-Werror
endif

all: watcher

nma.o:  hooks/nma.c
	gcc -c -fPIC hooks/nma.c -o nma.o    

watcher: main.cpp  nma.o
	$(CC) $(CFLAGS)   main.cpp nma.o `pkg-config opencv --cflags --libs sqlite3 --cflags --libs --cflags --libs glib-2.0` -lssl -lcrypto  -o $@



clean:
	rm -r  watcher db


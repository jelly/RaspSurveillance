ifeq ($(shell uname -m),armv6l)
 $(info Rpi)
 CC=g++
 CFLAGS=-O3  -mfpu=vfp -mfloat-abi=hard
else
 $(info native build)
 CC=clang++
 CFLAGS=-O0 -g  -pedantic -Werror
endif

all: watcher

watcher: main.cpp 
	$(CC) $(CFLAGS) main.cpp `pkg-config opencv --cflags --libs sqlite3 --cflags --libs --cflags --libs glib-2.0` -o $@



clean:
	rm -r  watcher db


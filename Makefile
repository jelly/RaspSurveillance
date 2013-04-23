ifeq ($(shell uname -m),armv6l)
 $(info Rpi)
 CC=g++
 CFLAGS=-O3  -mfpu=vfp -mfloat-abi=hard
else
 $(info native build)
 CC=clang++
 CFLAGS=-O2  -pedantic -Werror
endif

all: watcher 

watcher: opencv_functions.cpp main.cpp 
	$(CC) $(CFLAGS) main.cpp `pkg-config opencv --cflags --libs sqlite3 --cflags --libs` -o $@

clean:
	rm -r  watcher db


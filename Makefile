CC 		= gcc
INCLUDE 	= -I.
DEBUG 		=
CFLAGS 		= $(DEBUG) $(INCLUDE) -Winline -pipe
LDFLAGS 	= -L/usr/lib
LIB		= -lEBSTSD
#SOURCES  = $(wildcard *.c)
#INCLUDES = $(wildcard *.h)

SOURCES  = spidev-test.c spifunc.c bcm2835.c gpiofunc.c
INCLUDES = global.h spifunc.h bcm2835.h gpiofunc.h sdfunc.h
OBJECTS  = $(SOURCES:.c=*.o)
TARGET   = spidev-test
TARGET_SHARE_LIB = libEBSTSD.so

all: obj exe

obj: $(SOURCES) $(INCLUDES)
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(SOURCES)

exe:
	$(CC) $(LDFLAGS) -o  $(TARGET) $(OBJECTS) $(LIB) -lm

sharelib: 
	$(CC) -shared -o $(TARGET_SHARE_LIB) -fPIC -w sdfunc.c
	sudo cp ./$(TARGET_SHARE_LIB) /usr/lib
clean:
	rm -rf *.o
	rm -rf *.so

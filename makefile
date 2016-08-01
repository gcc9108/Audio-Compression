C=gcc
CFLAGS=-I.
DEPS = wave.h

# the build target executable:
TARGET = main


main: $(TARGET).o wav.o
	$(CC) -o $(TARGET) $(TARGET).o wav.o -I.

clean:
	rm -f *.o $(TARGET)


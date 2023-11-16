#Makefile
CC = g++
# CFLAGS = -Wall -Werror -Wmissing-prototypes
OBJS = build/i2cMux.o build/i2cScaleNAU7802.o build/tcpSocketTest.o
PROG = dist/tcpSocketTest
# LOCALLIBDIR = /usr/local/lib
# LDFLAGS = -L$(LOCALLIBDIR)
LDLIBS  = -li2c
INC1=-I../../Include

all: $(PROG)

# $(PROG): $(OBJS)
#     $(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)
$(PROG): $(OBJS)
	$(CC) $(INC) $(OBJS) -o $@ $(LDLIBS)

build/tcpSocketTest.o: tcpSocketTest.cpp
	$(CC) -c $(INC1) -o build/tcpSocketTest.o tcpSocketTest.cpp

build/i2cScaleNAU7802.o: ../../Source/i2cScaleNAU7802.cpp
	$(CC) -c $(INC1) -o build/i2cScaleNAU7802.o ../../Source/i2cScaleNAU7802.cpp

build/i2cMux.o: ../../Source/i2cMux.cpp
	$(CC) -c $(INC1) -o build/i2cMux.o  ../../Source/i2cMux.cpp


.PHONY: clean

clean:
	rm -f build/*~ build/*.o $(PROG) core a.out

run: $(PROG)
	./$(PROG) $(ARGS)

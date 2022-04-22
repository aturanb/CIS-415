CFLAGS=-W -Wall -I/usr/local/include
LDFLAGS=-L/usr/local/lib
PROGS=uspsv1 uspsv2 cpubound iobound

all: $(PROGS)

uspsv1: uspsv1.o p1fxns.o
	gcc $(LDFLAGS) -o $@ $^


uspsv2: uspsv2.o p1fxns.o
	gcc $(LDFLAGS) -o $@ $^

cpubound: cpubound.o
	gcc $(LDFLAGS) -o cpubound $^


iobound: iobound.o
	gcc $(LDFLAGS) -o iobound $^

clean:
	rm -f *.o $(PROGS)

uspsv1.o: uspsv1.c p1fxns.h
uspsv2.o: uspsv2.c p1fxns.h
cpubound.o: cpubound.c
iobound.o: iobound.c

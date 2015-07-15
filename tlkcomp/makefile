


ARCH=-m32
CFLAGS+=-Wall -g ${ARCH}

SRCS=tlkfile.c tlk_format.c tls_format.c tlkcomp.c
OBJS=tlkfile.o tlk_format.o tls_format.o tlkcomp.o

INCLUDES=tlkcomp.h
DEPS=${INCLUDES} ${SRCS}

%.o: %.c $(DEPS)
	$(CC)  -c -o $@ $< $(CFLAGS)

all: nwntlkcomp tlktest tlstest

nwntlkcomp:   ${INCLUDES} ${OBJS} tlkcompui.o
	gcc -o $@ tlkcompui.o $(CFLAGS) $(LIBS) ${OBJS} -I.

tlktest: ${INCLUDES} ${OBJS} tlktest.o
	gcc -o $@ tlktest.o $(CFLAGS) $(LIBS) ${OBJS} -I.

tlstest: ${INCLUDES} ${OBJS} tlstest.o
	gcc -o $@ tlstest.o $(CFLAGS) $(LIBS) ${OBJS} -I.

.PHONY: clean

clean:
	rm -f *.o *~ core tlktest tlstest nwntlkcomp

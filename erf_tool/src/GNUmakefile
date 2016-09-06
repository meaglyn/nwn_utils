##
## Definitions
##

CFLAGS = -m32 -g -O2 -Wall

SRCS = erf.c
OBJS = $(SRCS:.c=.o)

TARGETS = erf

##
## Rules
##

.SUFFIXES:
.SUFFIXES: .c .o

##
## Targets
##

default: $(TARGETS)

erf: $(OBJS)
	$(CC) ${CFLAGS} -o $@ $<

clean:
	rm $(TARGETS) *.o

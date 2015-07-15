!include <win32.mak>

##
## Definitions
##

CFLAGS = /DWIN32 /O2 /W3

SRCS = erf.c
OBJS = $(SRCS:.c=.obj)

TARGETS = erf.exe

##
## Rules
##

#.SUFFIXES:
#.SUFFIXES: .c .obj

##
## Targets
##

default: $(TARGETS)

erf.exe: $(OBJS)
	$(link) /nologo /out:$@ $(OBJS) setargv.obj

clean:
	-del *.exe *.obj

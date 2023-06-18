#!/bin/sh

if [ "${NWNTOOLS}x" == "x" ] ; then
	#NWNTOOLS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd -P )"  # does not work if file itself is a link
	#NWNTOOLS=`dirname $(realpath  $0)`   # this one works
	NWNTOOLS=`dirname $(readlink -f $0)`
	#echo Environement variable NWNTOOLS unset using ${NWNTOOLS}
fi

if [ -f ${NWNTOOLS}/tool_cfg ] ; then
	source ${NWNTOOLS}/tool_cfg
else
	echo unable to find config file ${NWNTOOLS}/tool_cfg
	exit 1
fi

#TODO - these should be elsewhere
rm -f *.ncs
rm -f *~

# 0 for off, 1 for each file name, 2 for each full command line
VERBOSE=0

#CFLAGS="-s"
#CFLAGS="-qr"
#CFLAGS="-rq -s"
EXTRA_ARGS="$*"

if [ "${EXTRA_ARGS}x" != "x" ] ; then
    if [ ${EXTRA_ARGS:0:1} != "-" ] ; then
	echo ignoring extra args
	EXTRA_ARGS=""
    fi
fi

if [ "${EXTRA_ARGS}x" != "x" ] ; then
    echo extra_args = "$EXTRA_ARGS"
fi
FILES=`ls -1 *.nss 2>/dev/null`

echo using Compiler : $NWNCOMP

if [ "${FILES}x" == "x" ] ; then
    echo Nothing to compile here.
    exit 0
fi

if [ $VERBOSE -eq 0 ] ; then
    echo -n "Compiling scripts "
fi
c=0
for i in *.nss ; 
do
    if [ $VERBOSE -eq 1 ] ; then
	echo compiling $i
    fi
    if [ $VERBOSE -eq 2 ] ; then
	echo ${NWNCOMP} ${NWNFLAGS} ${NWNCPP} ${EXTRA_ARGS} -i ${INCDIR}  $i
    fi
    ${NWNCOMP} ${NWNFLAGS} ${NWNCPP} ${EXTRA_ARGS} -i ${INCDIR} $i
    if [ ! $? -eq 0 ] ; then 
	exit 1
    fi
    if [ $VERBOSE -eq 0 ] ; then
	c=$(($c + 1))
	n=$(($c%5))
	if [ $n -eq 0 ] ; then 
	    echo -n "."
	fi
    fi
done
echo 

exit 0


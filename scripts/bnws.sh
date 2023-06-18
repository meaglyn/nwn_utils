#!/bin/sh
# compile the given nwscipt file.

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


#CFLAGS="-s"
#CFLAGS="-qr"
#CFLAGS="-rq -s"
FILES="$*"

if [ "${FILES}x" == "x" ] ; then
    echo "No file given!"
    exit 1
fi

#echo using Compiler : $NWNCOMP

#echo -n "Compiling scripts "
#echo ${NWNCOMP} ${NWNFLAGS} ${NWNCPP} ${EXTRA_ARGS} -i ${INCDIR}  ${FILES}
echo compiling ${FILES}
${NWNCOMP} ${NWNFLAGS} ${NWNCPP} ${EXTRA_ARGS} -i ${INCDIR} ${FILES}
    if [ ! $? -eq 0 ] ; then 
	exit 1
    fi

exit 0


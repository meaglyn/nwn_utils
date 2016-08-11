#!/bin/sh

if [ "${NWNTOOLS}x" == "x" ] ; then
	#NWNTOOLS="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd -P )"  # does not work if file itself is a link
	#NWNTOOLS=`dirname $(realpath  $0)`   # this one works
	NWNTOOLS=`dirname $(readlink -f $0)`
	echo Environement variable NWNTOOLS unset using ${NWNTOOLS}
fi

if [ -f ${NWNTOOLS}/tool_cfg ] ; then
	source ${NWNTOOLS}/tool_cfg
else
	echo unable to find config file ${NWNTOOLS}/tool_cfg
	exit 1
fi

rm -f *.ncs
rm -f *~

#CFLAGS="-s"
#CFLAGS="-qr"
#CFLAGS="-rq -s"

FILES=`ls -1 *.nss 2>/dev/null`

echo using Compiler : $NWNCOMP

if [ "${FILES}x" == "x" ] ; then
    echo Nothing to compile here.
    exit 0
fi

echo -n "Compiling scripts "
c=0
for i in *.nss ; 
do  
    #echo ${NWNCOMP} -i ${INCDIR} $i
    ${NWNCOMP} ${NWNFLAGS} ${NWNCPP} -i ${INCDIR} $i
    if [ ! $? -eq 0 ] ; then 
	exit 1
    fi
    c=$(($c + 1))
    n=$(($c%5))
    if [ $n -eq 0 ] ; then 
	echo -n "."
    fi
done
echo 

exit 0


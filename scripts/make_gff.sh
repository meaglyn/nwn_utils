#!/bin/sh

# Convert all the YML format files in the current directory back to gff.
# Run from the work directory. This deletes the yml files.

if [ "${NWNTOOLS}x" == "x" ] ; then
	NWNTOOLS=`dirname $(readlink -f $0)`
	echo Environement variable NWNTOOLS unset using ${NWNTOOLS}
fi

if [ -f ${NWNTOOLS}/tool_cfg ] ; then
	source ${NWNTOOLS}/tool_cfg
else
	echo unable to find config file ${NWNTOOLS}/tool_cfg
	exit 1
fi


usage() {
    echo "Usage : make_gff.sh"
    exit 1 
}

if [ ! $# -eq 0 ] ; then
    usage    
fi

rm -f *~ \#* 

YMLFILES=`find . -name \*.yml`
#echo ${YMLFILES}

for i in ${YMLFILES} ; do
    #echo $i
    j=`echo $i | sed 's/\(.*\)\..*/\1/'`
    echo ${NWNGFF} -i $i -l yaml -o $j -k gff
    RES=`${NWNGFF} -i $i -l yaml -o $j -k gff`
    if [ "$?" -ne 0 ] ; then
	echo ERROR: ${RES}
	exit 1
    fi
    rm -f $i
done




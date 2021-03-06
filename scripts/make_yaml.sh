#!/bin/sh

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

for i in `ls -1` ; do
    E=`echo "$i" | tail -c -4 `
    OKAY=false
    for j in ${EXTS} ; do 

	if [ $j = $E ] ; then
	    OKAY=true
	fi
    done

    # check if we should truncate the floats
    if [ "$E" = "are" -o "$E" = "git" -o "$E" = "utc" -o "$E" = "uts" ] ; then
	TRUNCATE="-r ${TRUNKSCRIPT}"
    else
	TRUNCATE=
    fi

    if [ "$OKAY" = "false" ] ; then
	if [ "$E" = "nss" -o "$E" = "xml" -o "$E" = "yml" ] ; then
	    #echo skipping $i
	    FOO=bar
	else
	    echo unknown extension $E for file $i
	fi
	continue
    fi
    
#    echo $i 
    echo ${NWNGFF} -i $i -l gff -o ${i}.yml -k ${OUTFORMAT} ${TRUNCATE} 
    ${NWNGFF} -i $i -l gff -o ${i}.yml -k ${OUTFORMAT} ${TRUNCATE} 
    if [ ! $? -eq 0 ] ; then 
	exit 1
    fi
    rm $i
done

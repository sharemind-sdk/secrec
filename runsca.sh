#!/bin/bash
D=$(cd `dirname $0` && pwd)
E=${D}/bin/sca
L=${D}/lib

if [ ! -e "$E" ]; then
    echo "$E does not appear to be built." >&2
    Q=${D}/scc.pro
    if [ ! -e "$Q" ]; then
        echo "In addition no project file was found in $D" >&2
        exit 1
    fi
    M=${D}/Makefile
    echo -n "You should probably run " >&2
    if [ ! -e "$M" ]; then
        echo -n "\"qmake\" and " >&2
	fi
	echo "\"make\" in ${D}." >&2
    exit 1
fi
LD_LIBRARY_PATH=${L} ${E} $*

#!/bin/bash
#
# Copyright (C) 2015 Cybernetica
#
# Research/Commercial License Usage
# Licensees holding a valid Research License or Commercial License
# for the Software may use this file according to the written
# agreement between you and Cybernetica.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU
# General Public License version 3.0 as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL included in the
# packaging of this file.  Please review the following information to
# ensure the GNU General Public License version 3.0 requirements will be
# met: http://www.gnu.org/copyleft/gpl-3.0.html.
#
# For further information, please contact us at sharemind@cyber.ee.
#

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

#!/bin/bash
#
#  This file is a part of the Sharemind framework.
#  Copyright (C) Cybernetica AS
#
#  All rights are reserved. Reproduction in whole or part is prohibited
#  without the written consent of the copyright owner. The usage of this
#  code is subject to the appropriate license agreement.
#

EXIT_FAILURE=1
EXIT_SUCCESS=0

if [ -z "$TMPDIR" ]; then
  echo 'Error: $TMPDIR not properly set in environment!' >&2
  exit $EXIT_FAILURE
fi
TMPTEMPLATE="${TMPDIR}/`basename $0`.XXXXXX"

PREFIX=

OLDPATH=`pwd`
SCRIPT_FULLNAME="$0"
SCRIPT_PATH="`dirname \"$0\"`"

TAR=tar
type gnutar >/dev/null 2>&1 && TAR=gnutar

function usage {
  SCRIPT_NAME=${SCRIPT_FULLNAME:${#SCRIPT_PATH}}
  SCRIPT_NAME=${SCRIPT_NAME:1}
  echo
  echo 'Usage:'
  echo
  echo "    $SCRIPT_NAME --prefix=<prefix>"
  echo
  echo 'This script archieves the git repository in the path where it resides,'
  echo 'including all submodules in ext/* directories. All archieved paths are'
  echo 'prefixed with "<prefix>/" and output is written to "<prefix>.tar.bz2"'
  echo 'relative to the current directory.'
  echo
}

# Handle command-line arguments:
shopt -s extglob
while test $# -gt 0; do
case $1 in

        --prefix=* )
            PREFIX="${1:9}"
            shift
            ;;

        -h | -? | --help )
            usage
            exit $EXIT_SUCCESS
            ;;

        * )
            echo "Invalid argument: $1" >&2
            usage
            exit $EXIT_FAILURE
            ;;
    esac
done

if [ -z "$PREFIX" ]; then
  echo 'Error: --prefix not specified!' >&2
  usage
  exit $EXIT_FAILURE
fi

OUTFILE="${PREFIX}.tar.bz2"

if [ -e "$OUTFILE" ]; then
  echo "Error: The file ${OUTFILE} already exists!" >&2
  usage
  exit $EXIT_FAILURE
fi

cd "$SCRIPT_PATH" # Go to script path

TMPOUTFILE=`mktemp ${TMPTEMPLATE}`
git archive "--prefix=${PREFIX}/" --format=tar HEAD -o "$TMPOUTFILE"

TMPSUBMODULEFILE=`mktemp ${TMPTEMPLATE}`
for SUBMODULEPATH in ext/*; do
  > "$TMPSUBMODULEFILE"
  cd "$SUBMODULEPATH" # Go to submodule path
  git archive --prefix="${PREFIX}/${SUBMODULEPATH}/" --format=tar HEAD -o "$TMPSUBMODULEFILE"
  cd - 2>&1 > /dev/null # Return from submodule path
  $TAR --concatenate "--file=$TMPOUTFILE" "$TMPSUBMODULEFILE"
done
rm -f "$TMPSUBMODULEFILE"

cd "$OLDPATH" # Return to caller path

bzip2 --best "$TMPOUTFILE" --stdout > "$OUTFILE"
rm -f "$TMPOUTFILE"

BYTES=`wc -c "$OUTFILE" | cut -d ' ' -f 1`
echo 'Details:'
echo "  Filename:  ${OUTFILE}"
echo "  File size: $BYTES bytes`echo $BYTES|awk '{s[2**30]="G";s[2**20]="M";s[1024]="k";for(x=2**30;x>=1024;x/=1024){if($1>=x){printf " (%.2f %sB)\n",$1/x,s[x];break}}}'`"
echo "  MD5SUM:    `md5sum \"${OUTFILE}\" |cut -b -32`"
echo "  SHA1SUM:   `sha1sum \"${OUTFILE}\" |cut -b -40`"
echo "  SHA256SUM: `sha256sum \"${OUTFILE}\" |cut -b -64`"

exit $EXIT_SUCCESS

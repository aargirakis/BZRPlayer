#! /bin/sh
# 
# (C) 2003 benjamin gerard <ben@sashipa.com>
#
# Generate a generic doxygen configuration file
#

function transform
{
    name="`expr "$1" : '\(.*\)=.*' \| "$1"`"
    val="`expr "$1" : '.*=\(.*\)' \| ""`"
    echo "s¬${name}\([[:space:]]*\)=.*¬${name}\1= ${val}¬"
}

export line=

file="$1"
shift
i=0;

# Check for even number of arguments
if [ "`expr $# '%' 2`" != "0" ]; then
   echo "Invalid number of arguments : $#" 1>&2
   exit 254
fi

while [ $# -ne 0 ]; do
    if [ "$1" != "-D" ]; then
	echo "Invalid option expected [-D], got [$1]" 1>&2
	exit 253
    fi
    line[$i]="-e"
    i=`expr $i + 1`
    shift

    if [ "`expr "$1" : '[[:alpha:]_]\{4,\}=.*'`" == "0" ]; then
	echo "Invalid substitution string." 1>&2
	echo "$1" 1>&2
	exit 252
    fi
    line[$i]="`transform "$1"`"
    i=`expr $i + 1`
    shift
done

sed  "${line[@]}" > "${file}"
if [ $? -ne 0 ]; then
    rm -f "${file}"
    exit 255
fi


echo "Configure the scripts"

echo "Autotools are stupid" > README
libtoolize
aclocal
autoconf
autoheader
automake -c -a

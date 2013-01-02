#!/bin/sh

#
# ZarBee - addfile.sh
#
# Author: sast
# Created on: Sun Feb 13 20:37:52 CET 2005
#
# this script creates some skeleton files for a given name
#

# header definitions:

echo -e "/*\n * ZarBee - $1.hh\n *" > $1.hh
echo " * Author: ${USER}" >> $1.hh
echo -n " * Created on:" `date` >> $1.hh
echo -e " *\n */" >> $1.hh

echo >> $1.hh

echo "#ifndef _$1_hh_" >> $1.hh
echo "#define _$1_hh_" >> $1.hh

echo >> $1.hh

echo -e "#include \"bee_base.hh\"" >> $1.hh

echo -e "\n\n" >> $1.hh

echo "#endif // _$1_hh_" >> $1.hh

# source definitions:

echo -e "/*\n * ZarBee - $1.cc\n *" > $1.cc
echo " * Author: ${USER}" >> $1.cc
echo -n " * Created on:" `date` >> $1.cc
echo -e " *\n */" >> $1.cc

echo >> $1.cc

echo -e "#include \"$1.hh\"" >> $1.cc

echo -e "\n\n" >> $1.cc

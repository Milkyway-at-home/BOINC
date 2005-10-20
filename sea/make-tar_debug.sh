#!/bin/sh

rm -fr BOINC_debug/.CVS
mkdir BOINC_debug
mkdir BOINC_debug/locale
find ../locale/client -name '*.mo' | cut -d '/' -f 4 | awk '{print "BOINC_debug/locale/"$0}' | xargs mkdir
find ../locale/client -name '*.mo' | cut -d '/' -f 4,5 | awk '{print "cp \"../locale/client/"$0"\" \"BOINC_debug/locale/"$0"\""}' | bash
tar cvvf sea_debug.tar BOINC_debug/


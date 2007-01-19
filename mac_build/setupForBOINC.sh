#!/bin/sh

# Berkeley Open Infrastructure for Network Computing
# http://boinc.berkeley.edu
# Copyright (C) 2005 University of California
#
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation;
# either version 2.1 of the License, or (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# To view the GNU Lesser General Public License visit
# http://www.gnu.org/copyleft/lesser.html
# or write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
# Master script to build Universal Binary libraries needed by BOINC:
# curl-7.15.5, jpeg-6b and wxMac-2.8.0
#
# by Charlie Fenton 7/21/06
# Updated for wx-Mac 2.8.0 1/19/07
#
# Download these three packages and place them in a common parent 
# directory with the BOINC source tree.
#
## In Terminal, cd to the mac_build directory of the boinc tree; for 
## example:
##     cd [path]/boinc/mac_build/
## then run this script:
##     source setupForBoinc.sh [ -clean ]
#
# the -clean argument will force a full rebuild of everything.
#
# This script will work even if you have renamed the boinc/ directory
#

if [ "$1" = "-clean" ]; then
  doclean="-clean"
else
  doclean=""
fi


echo ""
echo "----------------------------------"
echo "------- BUILD CURL-7.15.5 --------"
echo "----------------------------------"
echo ""

SCRIPT_DIR=`pwd`

if [ ! -f ../../curl-7.15.5/lib/url.c.orig ]; then
patch -bN ../../curl-7.15.5/lib/url.c ../curl/patches/7.15.5.Socks/url_c.patch
else
echo "url.c already patched"
fi

cd ../../curl-7.15.5/
if [  $? -ne 0 ]; then return 1; fi
source "${SCRIPT_DIR}/buildcurl.sh" ${doclean}
if [  $? -ne 0 ]; then return 1; fi

echo ""
echo "----------------------------------"
echo "--------- BUILD JPEG-6B ----------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cd ../../jpeg-6b/
if [  $? -ne 0 ]; then return 1; fi
source "${SCRIPT_DIR}/buildjpeg.sh" ${doclean}
if [  $? -ne 0 ]; then return 1; fi

echo ""
echo "----------------------------------"
echo "------- BUILD wxMac-2.8.0 --------"
echo "----------------------------------"
echo ""

cd "${SCRIPT_DIR}"

cp -fpR wxMac-BOINC.xcodeproj ../../wxMac-2.8.0/src/

cd ../../wxMac-2.8.0/
if [  $? -ne 0 ]; then return 1; fi
source "${SCRIPT_DIR}/buildWxMac.sh" ${doclean}
if [  $? -ne 0 ]; then return 1; fi

cd "${SCRIPT_DIR}"
return 0

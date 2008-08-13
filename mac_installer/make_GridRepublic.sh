#!/bin/csh

## Berkeley Open Infrastructure for Network Computing
## http://boinc.berkeley.edu
## Copyright (C) 2005 University of California
##
## This is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation;
## either version 2.1 of the License, or (at your option) any later version.
##
## This software is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU Lesser General Public License for more details.
##
## To view the GNU Lesser General Public License visit
## http://www.gnu.org/copyleft/lesser.html
## or write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

##
# Script to convert Macintosh BOINC installer to GridRepublic Desktop installer
# updated 2/21/08 by Charlie Fenton
##

## Usage:
## First put the following files into a working directory:
##     the BOINC installer to be converted
##     the Uninstall BOINC application to be converted
##     GR_ReadMe.rtf
##     gridrepublic.icns
##     GR_install.icns
##     GR_uninstall.icns
##     COPYING
##     COPYRIGHT
##     gridrepublic.tiff (for screensaver)
##     gridrepublic_ss_logo (for screensaver)
##     skins directory containing GridRepublic skin (optional)
##
## cd to the working directory:
##
## Invoke this script with the three parts of version number as arguments.  
## For example, if the version is 3.2.1:
##     sh [path_to_this_script] 3 2 1
##
## This will create a directory "BOINC_Installer" in the parent directory of 
## the current directory
##
## For different branding, modify the following 9 variables:
PR_PATH="GR_Pkg_Root"
IR_PATH="GR_Installer_Resources"
SCRIPTS_PATH="GR_Installer_Scripts"
NEW_DIR_PATH="New_Release_GR_$1_$2_$3"
README_FILE="GR-ReadMe.rtf"
## BRANDING_FILE="GR-Branding"
BRANDING_INFO="BrandId=1"
ICNS_FILE="gridrepublic.icns"
INSTALLER_ICNS_FILE="GR_install.icns"
UNINSTALLER_ICNS_FILE="GR_uninstall.icns"
SAVER_SYSPREF_ICON="gridrepublic.tiff"
SAVER_LOGO="gridrepublic_ss_logo.png"
BRAND_NAME="GridRepublic"
MANAGER_NAME="GridRepublic Desktop"
LC_BRAND_NAME="gridrepublic"
SOURCE_PKG_PATH="BOINC Installer.app/Contents/Resources/BOINC.pkg/Contents"

if [ $# -lt 3 ]; then
echo "Usage:"
echo "   cd working_directory"
echo "   sh [path_to_this_script] major_version minor_version revision_number"
echo ""
echo "See comments at start of script for more info."
echo ""
exit 1
fi

pushd ./

if [ -f /Developer/usr/bin/packagemaker ]; then
    PACKAGEMAKER_VERSION=3
else
    PACKAGEMAKER_VERSION=2
fi

sudo rm -dfR "${IR_PATH}"
sudo rm -dfR "${PR_PATH}"
sudo rm -dfR "${SCRIPTS_PATH}"

mkdir -p "${IR_PATH}"
mkdir -p "${PR_PATH}"

sudo rm -dfR "${NEW_DIR_PATH}/"

mkdir -p "${NEW_DIR_PATH}/"
mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal"
mkdir -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras"

cp -fp "${SOURCE_PKG_PATH}/Archive.pax.gz" "${PR_PATH}/"
cd "${PR_PATH}"
sudo gunzip ./Archive.pax.gz
sudo pax -r -pe -f Archive.pax
rm -df "Archive.pax"
cd ..

cp -fp "${SOURCE_PKG_PATH}/Resources/License.rtf" "${IR_PATH}/"
cp -fp "${README_FILE}" "${IR_PATH}/ReadMe.rtf"
# Update version number
sed -i "" s/"<VER_NUM>"/"$1.$2.$3"/g "${IR_PATH}/ReadMe.rtf"

if [ "$PACKAGEMAKER_VERSION" = "3" ]; then
    mkdir -p "${SCRIPTS_PATH}"
else
    SCRIPTS_PATH=${IR_PATH}
fi

# Create the installer's preinstall and preupgrade scripts from the standard preinstall script
# Older versions of BOINC installer did not use preinstall and preupgrade scripts, so check first
if [ -f "${SOURCE_PKG_PATH}/Resources/preinstall" ]; then
    cp -fp "${SOURCE_PKG_PATH}/Resources/preinstall" "${SCRIPTS_PATH}/"

    sed -i "" s/BOINCManager/"${MANAGER_NAME}"/g "${SCRIPTS_PATH}/preinstall"
    sed -i "" s/BOINCSaver/"${BRAND_NAME}"/g "${SCRIPTS_PATH}/preinstall"

    cp -fp "${SCRIPTS_PATH}/preinstall" "${SCRIPTS_PATH}/preupgrade"
fi

cp -fp "${SOURCE_PKG_PATH}/Resources/postinstall" "${SCRIPTS_PATH}/"
cp -fp "${SOURCE_PKG_PATH}/Resources/postupgrade" "${SCRIPTS_PATH}/"
cp -fpR "${SOURCE_PKG_PATH}/Resources/PostInstall.app" "${IR_PATH}/"
cp -fp "${SOURCE_PKG_PATH}/Resources/all_projects_list.xml" "${IR_PATH}/"

##### We've decided not to customize BOINC Data directory name for branding
#### mkdir -p "${PR_PATH}/Library/Application Support/${BRAND_NAME} Data"
#### mkdir -p "${PR_PATH}/Library/Application Support/${BRAND_NAME} Data/locale"

## Put Branding file into BOINC Data folder to make it available to screensaver
sudo echo ${BRANDING_INFO} > "${PR_PATH}/Library/Application Support/BOINC Data/Branding"

## If skins folder is present. copy it into BOINC Data folder
if [ -d "skins" ]; then
    cp -fR "skins" "${PR_PATH}/Library/Application Support/BOINC Data/"
fi

## Modify for Grid Republic
# Rename the Manager's bundle and its executable inside the bundle
sudo mv -f "${PR_PATH}/Applications/BOINCManager.app/" "${PR_PATH}/Applications/${MANAGER_NAME}.app/"
sudo mv -f "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/MacOS/BOINCManager" "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/MacOS/${MANAGER_NAME}"

# Update the Manager's info.plist, InfoPlist.strings files
sudo sed -i "" s/BOINCManager/"${MANAGER_NAME}"/g "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Info.plist"
sudo sed -i "" s/BOINCMgr.icns/"${ICNS_FILE}"/g "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Info.plist"
sudo sed -i "" s/BOINC/"${MANAGER_NAME}"/g "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace the Manager's BOINCMgr.icns file
sudo cp -fp "${ICNS_FILE}" "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/${ICNS_FILE}"
sudo rm -f "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/BOINCMgr.icns"

# Put Branding file in both Installer Package and Application Bundle
sudo echo ${BRANDING_INFO} > "${IR_PATH}/Branding"
sudo cp -fp "${IR_PATH}/Branding" "${PR_PATH}/Applications/${MANAGER_NAME}.app/Contents/Resources/Branding"

# Rename the screensaver bundle and its executable inside the bundle
sudo mv -f "${PR_PATH}/Library/Screen Savers/BOINCSaver.saver" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver"
sudo mv -f "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/MacOS/BOINCSaver" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/MacOS/${BRAND_NAME}"

# Update screensaver's info.plist, InfoPlist.strings files
sudo sed -i "" s/BOINCSaver/"${BRAND_NAME}"/g "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Info.plist"
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace screensaver's boinc.tiff or boinc.jpg file
sudo rm -f "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc.jpg"
sudo cp -fp "${SAVER_SYSPREF_ICON}" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc.tiff"

# Replace screensaver's boinc_ss_logo.png file
sudo rm -f "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc_ss_logo.png"
sudo cp -fp "${SAVER_LOGO}" "${PR_PATH}/Library/Screen Savers/${BRAND_NAME}.saver/Contents/Resources/boinc_ss_logo.png"

# Copy and rename the Uninstall application's bundle and rename its executable inside the bundle
sudo cp -fpR "Uninstall BOINC.app" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app"
sudo mv -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/MacOS/Uninstall BOINC" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/MacOS/Uninstall ${BRAND_NAME}"

# Update Uninstall application's info.plist, InfoPlist.strings files
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Info.plist"
sudo sed -i "" s/MacUninstaller.icns/"${UNINSTALLER_ICNS_FILE}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Info.plist"
#### sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace the Uninstall application's MacUninstaller.icns file
sudo cp -fp "${UNINSTALLER_ICNS_FILE}" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/${UNINSTALLER_ICNS_FILE}"
sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/MacUninstaller.icns"
# Remove the Uninstall application's resource file so it will show generic "Are you sure?" dialog
sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app/Contents/Resources/Uninstall BOINC.rsrc"

sudo chown -R root:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app"
sudo chmod -R 555 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/Uninstall ${BRAND_NAME}.app"

## Fix up ownership and permissions
sudo chown -R root:admin "${PR_PATH}"/*
sudo chmod -R u+rw,g+rw,o+r-w "${PR_PATH}"/*
sudo chmod 1775 "${PR_PATH}/Library"

sudo chown -R 501:admin "${PR_PATH}/Library/Application Support"/*
sudo chmod -R u+rw,g+r-w,o+r-w "${PR_PATH}/Library/Application Support"/*

sudo chown -R root:admin "${IR_PATH}"/*
sudo chmod -R u+rw,g+r-w,o+r-w "${IR_PATH}"/*
sudo chown -R root:admin "${SCRIPTS_PATH}"/*
sudo chmod -R u+rw,g+r-w,o+r-w "${SCRIPTS_PATH}"/*

sudo cp -fp "${IR_PATH}/ReadMe.rtf" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/ReadMe.rtf"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/ReadMe.rtf"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/ReadMe.rtf"
sudo cp -fp "COPYING" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYING"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYING"
sudo cp -fp "COPYRIGHT" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/"
sudo chown -R 501:admin "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYRIGHT"
sudo chmod -R 644 "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/extras/COPYRIGHT"

# Make temporary copies of Pkg-Info.plist and Description.plist for PackageMaker and update for this branding
sudo cp -fp "${SOURCE_PKG_PATH}/Info.plist" "${NEW_DIR_PATH}/Pkg-Info.plist"
sudo chown -R 501:admin "${NEW_DIR_PATH}/Pkg-Info.plist"
sudo chmod -R 666 "${NEW_DIR_PATH}/Pkg-Info.plist"
if [ -f "${SOURCE_PKG_PATH}/Resources/English.lproj/Description.plist" ]; then
    sudo cp -fp "${SOURCE_PKG_PATH}/Resources/English.lproj/Description.plist" "${NEW_DIR_PATH}"
else
    sudo cp -fp "${SOURCE_PKG_PATH}/Resources/en.lproj/Description.plist" "${NEW_DIR_PATH}"
fi
sudo chown -R 501:admin "${NEW_DIR_PATH}/Description.plist"
sudo chmod -R 666 "${NEW_DIR_PATH}/Description.plist"

sudo sed -i "" s/"BOINC Manager"/"${MANAGER_NAME}"/g "${NEW_DIR_PATH}/Pkg-Info.plist"
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/Pkg-Info.plist"
sudo sed -i "" s/"BOINC Manager"/"${MANAGER_NAME}"/g "${NEW_DIR_PATH}/Description.plist"
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/Description.plist"

# Copy the installer wrapper application "${BRAND_NAME} Installer.app"
sudo cp -fpR "BOINC Installer.app" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app"
sudo rm -dfR "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/BOINC.pkg"

# Update the installer wrapper application's info.plist, InfoPlist.strings files
sudo sed -i "" s/BOINC/"${BRAND_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Info.plist"
sudo sed -i "" s/MacInstaller.icns/"${INSTALLER_ICNS_FILE}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Info.plist"
## sed -i "" s/BOINC/"${MANAGER_NAME}"/g "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/English.lproj/InfoPlist.strings"

# Replace the installer wrapper application's MacInstaller.icns file
sudo cp -fp "${INSTALLER_ICNS_FILE}" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${INSTALLER_ICNS_FILE}"
sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/MacInstaller.icns"

# Rename the installer wrapper application's executable inside the bundle
sudo mv -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/MacOS/BOINC Installer" "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/MacOS/${BRAND_NAME} Installer"

# Build the installer package inside the wrapper application's bundle
if [ "$PACKAGEMAKER_VERSION" = "3" ]; then
    # Packagemaker Version 3
##  /Developer/usr/bin/packagemaker -r ../BOINC_Installer/Pkg_Root -e ../BOINC_Installer/Installer\ Resources/ -s ../BOINC_Installer/Installer\ Scripts/ -f mac_build/Pkg-Info.plist -t "BOINC Manager" -n "$1.$2.$3" -b -o ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_universal/BOINC\ Installer.app/Contents/Resources/BOINC.pkg
    /Developer/usr/bin/packagemaker -r "${PR_PATH}" -e "${IR_PATH}" -s "${SCRIPTS_PATH}" -f "${NEW_DIR_PATH}/Pkg-Info.plist" -t "${MANAGER_NAME}" -n "$1.$2.$3" -b -o "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg"
    # Remove TokenDefinitions.plist and IFPkgPathMappings in Info.plist, which would cause installer to find a previous copy of ${MANAGER_NAME} and install there
    sudo rm -f "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Resources/TokenDefinitions.plist"
    defaults delete "`pwd`/${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Info" IFPkgPathMappings
else
    # Packagemaker Version 2
##  /Developer/Tools/packagemaker -build -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_universal/BOINC\ Installer.app/Contents/Resources/BOINC.pkg -f ../BOINC_Installer/Pkg_Root -r ../BOINC_Installer/Installer\ Resources/ -i mac_build/Pkg-Info.plist -d mac_Installer/Description.plist -ds 
    /Developer/Tools/packagemaker -build -p "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg" -f "${PR_PATH}" -r "${IR_PATH}" -i "${NEW_DIR_PATH}/Pkg-Info.plist" -d "${NEW_DIR_PATH}/Description.plist" -ds 
fi

# Allow the installer wrapper application to modify the package's Info.plist file
sudo chmod u+w,g+w,o+w "${NEW_DIR_PATH}/${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal/${BRAND_NAME} Installer.app/Contents/Resources/${BRAND_NAME}.pkg/Contents/Info.plist"

# Remove temporary copies of Pkg-Info.plist and Description.plist
sudo rm ${NEW_DIR_PATH}/Pkg-Info.plist
sudo rm ${NEW_DIR_PATH}/Description.plist

# Remove temporary directories
sudo rm -dfR "${IR_PATH}"
sudo rm -dfR "${PR_PATH}"
sudo rm -dfR "${SCRIPTS_PATH}"

# Compress the products
cd ${NEW_DIR_PATH}
zip -rqy ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal.zip ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal
##### We've decided not to create branded command-line executables; they are identical to standard ones
#### zip -rqy ${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin.zip ${LC_BRAND_NAME}_$1.$2.$3_universal-apple-darwin
##### We've decided not to create branded symbol table file; it is identical to standard one
#### zip -rqy ${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables.zip ${LC_BRAND_NAME}_$1.$2.$3_macOSX_SymbolTables

# Force Finder to recognize changed icons by deleting the uncompressed products and expanding the zip file 
sudo rm -dfR ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal
open ${LC_BRAND_NAME}_$1.$2.$3_macOSX_universal.zip

popd
exit 0

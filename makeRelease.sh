#!/bin/bash

cd "`dirname "$0"`"

# Read version number
eval `awk '/VC_VERSION_NUMBER 0x[0-9]+/ { h=$3 }
END {
major=strtonum(substr(h, 1, 4))
minor=strtonum("0x" substr(h, 5, 2))
patch=strtonum("0x" substr(h, 7, 2)) / 2
printf "oldVersion=\"%d.%d.%d\"\n", major, minor, patch
printf "newVersion=\"%d.%d.%d\"\n", major, minor, patch + 1
}' include/Vc/version.h`
echo    "current version: $oldVersion"
echo -n "    new release: "
read -e -i "$newVersion" newVersion

versionString=$newVersion
versionNumber=`echo $newVersion | awk '{ split($0, v, "."); printf "0x%02x%02x%02x", v[1], v[2], v[3] * 2 }'`

# Update the version number
sed -i "s/^PROJECT_NUMBER         = .*\$/PROJECT_NUMBER         = $versionString/" doc/Doxyfile
sed -i \
	-e "s/VC_VERSION_STRING \".*\"\$/VC_VERSION_STRING \"$versionString\"/" \
	-e "s/VC_VERSION_NUMBER 0x.*\$/VC_VERSION_NUMBER $versionNumber/" \
	include/Vc/version.h
cat include/Vc/version.h
git commit doc/Doxyfile include/Vc/version.h -m"change version to $versionString"
git tag "$versionString"

# Create tarball
git archive --format=tar --prefix="Vc-$versionString/" "$versionString" | gzip > ../"Vc-$versionString.tar.gz"

# Update the version number of the after-release code
versionString="$versionString-dev"
versionNumber=`echo $versionNumber | awk '{ printf "0x%06x", (strtonum($0) + 1) }'`

sed -i "s/^PROJECT_NUMBER         = .*\$/PROJECT_NUMBER         = $versionString/" doc/Doxyfile
sed -i \
	-e "s/VC_VERSION_STRING \".*\"\$/VC_VERSION_STRING \"$versionString\"/" \
	-e "s/VC_VERSION_NUMBER 0x.*\$/VC_VERSION_NUMBER $versionNumber/" \
	include/Vc/version.h
git commit doc/Doxyfile include/Vc/version.h -m"change version to $versionString"

#!/bin/bash

# This script creates a new minor release of the ftduino package.

# By default with no argumentes given it will increase the PATCH
# level by one. If a version number is given then that version is
# being created or updated. If PATCH, MINOR or MAJOR are given as
# argument then these are increased.

# first find version sorted list of all existing releases
RDIRS=`ls -d releases/*.*.* | sort --version-sort`

echo "Existing releases:"
RELEASES=()
for r in $RDIRS; do
    r=`echo $r | cut -d/ -f2 -`
    echo "   $r"
    RELEASES+=($r)
done

function contains() {
    local n=$#
    local value=${!n}
    for ((i=1;i < $#;i++)) {
        if [ "${!i}" == "${value}" ]; then
            echo "y"
            return 0
        fi
    }
    echo "n"
    return 1
}

function vercomp() {
    if [[ $1 == $2 ]]
    then
	echo "equal"
        return 0
    fi
    local IFS=.
    local i ver1=($1) ver2=($2)
    # fill empty fields in ver1 with zeros
    for ((i=${#ver1[@]}; i<${#ver2[@]}; i++))
    do
        ver1[i]=0
    done
    for ((i=0; i<${#ver1[@]}; i++))
    do
        if [[ -z ${ver2[i]} ]]
        then
            # fill empty fields in ver2 with zeros
            ver2[i]=0
        fi
        if ((10#${ver1[i]} > 10#${ver2[i]}))
        then
	    echo "bigger"
            return 0
        fi
        if ((10#${ver1[i]} < 10#${ver2[i]}))
        then
	    echo "smaller"
            return 0
        fi
    done
    echo "equal"
    return 0
}

# check for the latest version available
LATEST=${RELEASES[-1]}
# cur latest version into parts
MAJOR=`echo $LATEST | cut -d. -f1`
MINOR=`echo $LATEST | cut -d. -f2`
PATCH=`echo $LATEST | cut -d. -f3`

# compute the version to be created
if [ $# -gt 0 ]; then
    # check if parameter is a valid version
    if [[ $1 =~ ^[0-9]+.[0-9]+.[0-9]+$ ]]; then
	NEWRELEASE=$1
    elif [[ "${1,,}" == "patch" ]]; then
	PATCH=$((PATCH+1))
	NEWRELEASE=$MAJOR.$MINOR.$PATCH
    elif [[ "${1,,}" == "minor" ]]; then
	MINOR=$((MINOR+1))
	NEWRELEASE=$MAJOR.$MINOR.0
    elif [[ "${1,,}" == "patch" ]]; then
	MAJOR=$((MAJOR+1))
	NEWRELEASE=$MAJOR.0.0
    else
	echo "ERROR: Invalid argument!"
	echo "Argument must either be a valid release version or"
	echo "one of 'patch', 'minor' or 'major'!"
	[[ "$0" = "$BASH_SOURCE" ]] && exit 1 || return 1
    fi    
else
    # no argument given: Just increase patch level
    PATCH=$((PATCH+1))
    NEWRELEASE=$MAJOR.$MINOR.$PATCH
fi

# check if the release to be created already exists
if [ $(contains "${RELEASES[@]}" "$NEWRELEASE") == "y" ]; then
    echo "A release with version $NEWRELEASE already exists!"
    read -p "Are you sure you want to overwrite an existing release? " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
	echo "Aborting ..."
	[[ "$0" = "$BASH_SOURCE" ]] && exit 1 || return 1
    fi
else
    # check if the new release is not the newest one
    if [ $(vercomp "$NEWRELEASE" "$LATEST") != "bigger" ]; then
	echo "There are already releases with a higher version number than $NEWRELEASE!"
	read -p "Are you sure you want to create an intermediate release? " -n 1 -r
	echo
	if [[ ! $REPLY =~ ^[Yy]$ ]]; then
	    echo "Aborting ..."
	    [[ "$0" = "$BASH_SOURCE" ]] && exit 1 || return 1
	fi
    fi
fi

echo "Creating release $NEWRELEASE ..."

ARCHIVEDIR=releases/$NEWRELEASE
ARCHIVE=$ARCHIVEDIR/ftduino-$NEWRELEASE.zip

if [ ! -d "$ARCHIVEDIR" ]; then 
    mkdir $ARCHIVEDIR
fi

echo "Creating $ARCHIVE ..."
zip -qr $ARCHIVE ftduino

# finally update the package index
echo "Updating package index ..."
./mk_package_index.py

echo "done."


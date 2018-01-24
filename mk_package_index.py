#! /usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Python script to build ftduino package file
#

# ---------- configuraton -----------

PACKAGE_NAME = "ftduino"

MAINTAINER = "Till Harbaum"
WEBSITE = "https://ftduino.de"
EMAIL = "info@ftduino.de"
HELP = "http://ftduino.de/issues"
BOARDNAME = "ftDuino fischertechnik compatible controller"
ARCHITECTURE = "avr"
PLATFORM_NAME = "ftDuino"

GITHUB = "https://raw.githubusercontent.com/harbaum/ftduino/master/"
INDEXJSON = "package_" + PACKAGE_NAME + "_index.json"
RELEASES_PATH = "releases"

# -----------------------------------

import json, hashlib, os

index = { }
index["packages"] = [ ]

# currently there's only one package for the ftDuino
ftduino = { }
ftduino["name"] = PACKAGE_NAME
ftduino["maintainer"] = MAINTAINER
ftduino["websiteURL"] = WEBSITE
ftduino["email"] = EMAIL
ftduino["help"] = { "online": WEBSITE }

# collect releases
versions = [ ]
rlist = os.listdir(RELEASES_PATH)
for r in rlist:
    release = os.path.join(RELEASES_PATH, r)
    if os.path.isdir(release):
        versions.append(r)

releases = [ ]
for version in versions:
    print("Including version", version)
    
    release = { }
    release["name"] = PLATFORM_NAME
    release["architecture"] = ARCHITECTURE
    release["version"] = version
    release["category"] = PLATFORM_NAME

    archivefilename = "ftduino-" + version + ".zip"
    archivepath = RELEASES_PATH + "/" + version + "/" + archivefilename

    release["url"] = GITHUB + archivepath
    release["archiveFileName"] = archivefilename

    # get file hash and size
    with open(archivepath, 'rb') as f:
        release["checksum"] = "SHA-256:" + hashlib.sha256(f.read()).hexdigest()
    release["size"] = str(os.stat(archivepath).st_size)

    release["help"] = { "online": HELP }
    release["boards"] = [ { "name": BOARDNAME } ]
    releases.append(release)

ftduino["platforms"] = releases
ftduino["tools"] = [ ]
index["packages"].append( ftduino )

print("Writing", INDEXJSON)
with open(INDEXJSON, 'w') as outfile:
    json.dump(index, outfile, indent=4, ensure_ascii=False)

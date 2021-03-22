#!/bin/bash

[ -z "$1" ] && exit 2

# alternatively a lookup of filename  to package name

hash=$(echo -n "$1" | md5sum | tr '[a-f0-9]' '[A-FG-Z]')
echo "ZZ"${hash:1:6}

exit 0

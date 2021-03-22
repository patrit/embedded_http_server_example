#!/bin/bash

curl -f -s -m 1 http://127.0.0.1:8080/ping && exit 0

# in case there was a timeout e.g. request is being processed
# try to find out if server is still ok
[ -f "/tmp/lastHttpRequest" ] || exit 1

lastreq=$(cat /tmp/lastHttpRequest)
[ -z "$lastreq" ] && exit 1

timeout=$(($lastreq + 3 * 60)) # min timeout
[ "$timeout" -le $(date  +"%s") ] && exit 1

exit 0

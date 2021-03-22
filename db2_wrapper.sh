#!/bin/bash

[ "$#" -lt "1" ] && echo "ERROR: No DB2 command given" && exit 1
[ -z "$DBNAME" ] && DBNAME=SAMPLE
[ -z "$DBA" ] && DBA=dude
[ -z "$DBA_PASSWD" ] && DBA_PASSWD=pw42
cmd="$*"
isCygwin="$(uname -s | grep -i cygwin)"
ret=0

assertDBconnected()
{
   if [ ! -z "$isCygwin" ]; then
      cmd="db2 connect to $DBNAME user $DBA using $DBA_PASSWD && db2 set schema BALSY && "$cmd" && db2 connect reset"
   else
     db2 set connection $VWSDB
     if [ "$?" -ne "0" ]; then
        cmd="db2 connect to $DBNAME user $DBA using $DBA_PASSWD && db2 set schema BALSY && "$cmd
     fi
   fi
}

execCmd()
{
   if [ ! -z "$isCygwin" ]; then
      db2cmd -c -w -i "$cmd"
   else
      eval $cmd
   fi
   ret=$?
}

assertDBconnected
execCmd
exit $ret

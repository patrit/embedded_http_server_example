#!/bin/bash

# Add local user
# Either use the LOCAL_USER_ID if passed in at runtime or
# fallback

[ "$STARTDBSERVER" == "true" ] && su -c "db2start" - db2admin

/catalogdb.sh

USER_ID=${LOCAL_USER_ID:-1000}

echo "Starting with UID : $USER_ID"
useradd --shell /bin/bash -u $USER_ID -o -c "" -m balsy
adduser balsy db2grp1
echo "export PATH=$PATH" >> /home/balsy/.profile
echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH" >> /home/balsy/.profile
echo "export DB2INSTANCE=$DB2INSTANCE" >> /home/balsy/.profile


su -l -c "$@" - balsy

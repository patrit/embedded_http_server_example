#!/bin/sh


# generate configs
[ -z $DB2_DB ] && DB2_DB=SAMPLE
[ -z $DB2_DBA ] && DB2_DBA=dude
[ -z $DB2_DBA_PASSWD ] && DB2_DBA_PASSWD=pw42
[ -z $DB2_HOST ] && export DB2_HOST=db2
[ -z $DB2_PORT ] && export DB2_PORT=50000

# when restarts clean database catalog
su -c "db2 uncatalog database SAMPLE" - dude
su -c "db2 uncatalog node db2node" - dude

# database catalog
su -c "db2 catalog tcpip node db2node remote $DB2_HOST server $DB2_PORT" - dude
su -c "db2 catalog database $DB2_DB as SAMPLE at node db2node" - dude
su -c "db2 terminate" - dude

export LD_LIBRARY_PATH=/opt/ibm/db2/lib64/
export LC_ALL=C.ISO-8859-1

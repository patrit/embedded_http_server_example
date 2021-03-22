#!/bin/bash

set -e

# generate configs
[ -z $DB2_DB ] && export DB2_DB=SAMPLE
[ -z $DB2_DBA ] && export DB2_DBA=dude
[ -z $DB2_DBA_PASSWD ] && export DB2_DBA_PASSWD=pw42
[ -z $DB2_HOST ] && export DB2_HOST=db2
[ -z $DB2_PORT ] && export DB2_PORT=50000
[ -z $DB2INSTANCE ] && export DB2INSTANCE=dude

/home/db2admin/sqllib/db2profile
export LD_LIBRARY_PATH=/opt/ibm/db2/lib64/
/opt/demo/db2catalog.sh

DBNAME=$DB2_DB DBA=$DB2_DBA DBA_PASSWD=$DB2_DBA_PASSWD /opt/demo/server --port=8080

#!/bin/bash

set -e

[ -z $DB2_DB ] && export DB2_DB=SAMPLE
[ -z $DB2_DBA ] && export DB2_DBA=dude
[ -z $DB2_DBA_PASSWD ] && export DB2_DBA_PASSWD=pw42
[ -z $DB2_HOST ] && export DB2_HOST=db2
[ -z $DB2_PORT ] && export DB2_PORT=50000
[ -z $DB2INSTANCE ] && export DB2INSTANCE=dude

/home/db2admin/sqllib/db2profile
/opt/demo/db2catalog.sh
/opt/demo/drop_packages.sh
/opt/demo/bindsql.sh

exit $?

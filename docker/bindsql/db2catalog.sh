#!/bin/sh
/home/db2admin/sqllib/db2profile
LD_LIBRARY_PATH=/opt/ibm/db2/lib64/
PATH=/opt/ibm/db2/bin/:$PATH
# when restarts clean database catalog
db2 uncatalog database SAMPLE
db2 uncatalog node db2node

# database catalog
db2 catalog tcpip node db2node remote $DB2_HOST server $DB2_PORT
db2 catalog database $DB2_DB as SAMPLE at node db2node
db2 terminate


#!/bin/bash

/home/db2admin/sqllib/db2profile
LD_LIBRARY_PATH=/opt/ibm/db2/lib64/
PATH=/opt/ibm/db2/bin/:$PATH

echo "Drop deprecated packages"
cd /tmp
db2 "connect to SAMPLE user $DB2_DBA using $DB2_DBA_PASSWD" > resp
if [ -z "$(cat resp | grep "z/OS")" ]
then
   db2 "select distinct pkgschema,pkgname,pkgversion from syscat.packages A where 10 <= (select count(*) from syscat.packages B where pkgname = A.pkgname and last_bind_time > A.last_bind_time) and pkgname like 'BY%' and pkgversion != ''" | grep ' BY'  > drop_packages
else
   db2 "select distinct collid,name,version from sysibm.syspackage A where 10 <= (select count(*) from sysibm.syspackage B where name = A.name and bindtime > A.bindtime) and name like 'BY%' and version != ''"| grep ' BY'  > drop_packages
fi
db2 connect reset
echo "db2 connect to SAMPLE user $DB2_DBA using $DB2_DBA_PASSWD" > inject.cmd
cat drop_packages | awk -v SQ="'" '{ print "db2 "SQ"DROP PACKAGE "$1"."$2" VERSION \""$3"\""SQ"\n"}' >> inject.cmd
echo "db2 connect reset" >> inject.cmd
bash inject.cmd >/dev/null
rm -f inject.cmd drop_packages resp

#!/bin/bash

/home/db2admin/sqllib/db2profile
LD_LIBRARY_PATH=/opt/ibm/db2/lib64/
PATH=/opt/ibm/db2/bin/:$PATH

cd /opt/demo/bind

db2 "connect to SAMPLE user $DB2_DBA using $DB2_DBA_PASSWD"
bndfile=$(ls *.bnd | sort -n | head -1)
pkgversion=$(db2bfd -b $bndfile | grep Version | cut -d'"' -f2)
db2 -x "select pkgname from syscat.packages where pkgversion = '"$pkgversion"' and valid='Y'" > .pkgnames
for i in $(ls *.bnd); do
   echo -n "Bind $i"

   # test if version has to be binded
   pkgname=$(db2bfd -b $i | grep "App Name" | cut -d'"' -f2)
   [ -n "$(grep $pkgname .pkgnames)" ] && echo ", already applied" && continue

   if [ -z "$DB2_QUALIFIER" ]; then
      db2 "bind $i DATETIME ISO DEGREE ANY KEEPDYNAMIC YES DYNAMICRULES BIND VALIDATE BIND" 2>&1 1>/tmp/res
   else
      db2 "bind $i DATETIME ISO QUALIFIER "$DB2_QUALIFIER" DEGREE ANY KEEPDYNAMIC YES DYNAMICRULES BIND VALIDATE BIND" 2>&1 1>/tmp/res
   fi
   rc=$?
   echo ", ret="$rc
   [ $rc -ne 0 ] && cat /tmp/res
   [ $rc -ne 0 -a $rc -ne 2 ] && db2 "connect reset" && exit $rc
done
db2 "connect reset"

exit 0

#!/bin/sh
#
# This test copies a file to the SE and then asks for its TURL using 
# the root protocol. It verifies that the returned TURL is valid.
# The copy is performed in the srm2_d0t1 space token.
#
if test $# -le 0; then
  echo "Usage: $0 <srm v2.2 full endpoint>"
  echo "Please provide srm v2.2 full endpoint"
  exit 1
else
   srmv2=$1
fi
#
VO="dteam"
if test -z ${SPACE}; then
   space="srm2_d0t1"
else
   space=${SPACE}
fi
echo "Space token description = ${space}"
#
# Copy the file
#
cdate=`date '+%Y%m%d-%H%M%S'`
fil="${srmv2}/lcg-util/test-${cdate}.txt"
lcg-cp -b --vo ${VO} -D srmv2 -U srmv2 -S ${space} -v file:/etc/hosts ${fil}
result=$?
if [ $result -ne 0 ] ; then
  echo "ERROR: File ${fil} not copied"
  echo "Exit status: $result"
  exit ${result}
else
  echo "File ${fil} copied successfully"
fi
#
tresult=0
var="root rfio"
for prot in ${var} ; do
  lcg-gt -b -D srmv2 -T srmv2 -s ${space} ${fil} ${prot}
  result=$?
  if [ $result -ne 0 ] ; then
    echo "ERROR: TURL for file ${fil} with protocol ${prot} not provided"
    tresult=2
  fi
done
#
lcg-del -v -b -D srmv2 -T srmv2 --vo ${VO} ${fil}
dresult=$?
if [ $dresult -ne 0 ] ; then
  echo "ERROR: File ${fil} not deleted"
  echo "Exit status: $dresult"
else
  echo "File ${fil} deleted successfully"
fi
let fresult=$tresult+$dresult
exit $fresult
#

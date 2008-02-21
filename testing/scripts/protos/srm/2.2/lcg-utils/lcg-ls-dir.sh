#!/bin/sh
#
# This test lists the content of the top directory.
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
#
lcg-ls -b -T srmv2 -l -d ${srmv2}
result=$?
if [ $result -ne 0 ] ; then
  echo "ERROR: Directory ${srmv2} not listed"
else
  echo "Directory ${srmv2} listed successfully"
fi
#
exit $result
#

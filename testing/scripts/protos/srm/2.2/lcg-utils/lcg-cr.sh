#!/bin/sh
if test $# -le 0; then
  echo "Usage: $0 <srm v2.2 full endpoint>"
  echo "Please provide srm v2.2 full endpoint"
  exit 1
else
   srmv2=$1
fi
if test -z ${LCG_GFAL_INFOSYS}; then
  echo "Error: environmental variable LCG_GFAL_INFOSYS unset"
  exit 1
fi
#
# Find the catalogue to be used for this VO.
#
export LCG_GFAL_VO="dteam"
VO="$LCG_GFAL_VO"
catalog=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueService)(GlueServiceType=data-location-interface)(GlueServiceAccessControlRule='$LCG_GFAL_VO'))' GlueServiceEndpoint | sed -n '/^GlueServiceEndpoint: http:\/\// s/^GlueServiceEndpoint: http:\/\///gp' | sed -n '/:8085\// s/:8085\///gp'`
#
echo "The catalogue that will be used for registration is $catalog for VO=${LCG_GFAL_VO}"
export LCG_CATALOG_TYPE=lfc
export LFC_HOST=$catalog
#
# Use space token description
#
if test -z ${SPACE}; then
   space="srm2_d0t1"
else
   space=${SPACE}
fi
echo "Space token description = ${space}"
#
# Copy and register the file
#
cdate=`date '+%Y%m%d-%H%M%S'`
fil="${srmv2}/lcg-util/test-${cdate}.txt"
ofil="/tmp/$$"
lcg-cr -v -b -D srmv2 -T srmv2 -s ${space} file:/etc/group -d ${fil} > ${ofil} 2>&1
tresult=$?
cat ${ofil} 2>&1
if [ $tresult -ne 0 ] ; then
  echo "ERROR: Failed to copy and register file ${fil}"
  echo "Exit status: $tresult"
else
  echo "File ${fil} copied and registered successfully"
fi
guid=`sed -n -e 's/^guid://gp' ${ofil}`
/bin/rm ${ofil}
lcg-del --vo ${VO} -D srmv2 -T srmv2 -v ${fil}
#result=$?
#if [ $result -ne 0 ] ; then
#  echo "ERROR: Failed to delete file ${fil}"
#  echo "Exit status: $result"
#else
#  echo "File ${fil} deleted successfully"
#fi
#let fresult=$tresult+$result
exit $tresult
#

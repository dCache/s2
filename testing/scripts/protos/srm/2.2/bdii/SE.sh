#!/bin/sh
if test $# -le 0; then
  echo "Usage: $0 <srm v2 endpoint hostname>"
  echo "Please provide host name of srm-v2 endpoint"
  exit 1
else
   hst=$1
fi
if test -z ${LCG_GFAL_INFOSYS}; then
  echo "Error: environmental variable LCG_GFAL_INFOSYS unset"
  exit 1
fi
proc=$$
fil=/tmp/SE${proc}
Stat=0
ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueSE)(GlueSEUniqueID='$hst'))' GlueSEName GlueSEType GlueSEImplementationName GlueSEImplementationVersion GlueSEStatus GlueSETotalOnlineSize GlueSEUsedOnlineSize GlueSETotalNearlineSize GlueSEUsedNearlineSize GlueForeignKey > ${fil}
#
if test ! -s ${fil}; then
   echo "Error: No GlueSE object defined with GlueSEUniqueID=${hst}
"
   /bin/rm ${fil}
   Stat=2
else
#
   cat ${fil}
   var="SEName SEImplementationName SEImplementationVersion SEStatus"
   varp="SEName SEType SEImplementationName SEImplementationVersion SEStatus SETotalOnlineSize SEUsedOnlineSize SETotalNearlineSize SEUsedNearlineSize"
   SEName=`cat ${fil} | sed -n '/^GlueSEName:/ s/GlueSEName: \(.*\)/\1/gp'`
   SEType=`cat ${fil} | sed -n '/^GlueSEType:/ s/GlueSEType: \(.*\)/\1/gp'`
   SEImplementationName=`cat ${fil} | sed -n '/^GlueSEImplementationName:/ s/GlueSEImplementationName: \(.*\)/\1/gp'`
   SEImplementationVersion=`cat ${fil} | sed -n '/^GlueSEImplementationVersion:/ s/GlueSEImplementationVersion: \(.*\)/\1/gp'`
   SEStatus=`cat ${fil} | sed -n '/^GlueSEStatus:/ s/GlueSEStatus: \(.*\)/\1/gp'`
   SETotalOnlineSize=`cat ${fil} | sed -n '/^GlueSETotalOnlineSize:/ s/GlueSETotalOnlineSize: \(.*\)/\1/gp'`
   SEUsedOnlineSize=`cat ${fil} | sed -n '/^GlueSEUsedOnlineSize:/ s/GlueSEUsedOnlineSize: \(.*\)/\1/gp'`
   SETotalNearlineSize=`cat ${fil} | sed -n '/^GlueSETotalNearlineSize:/ s/GlueSETotalNearlineSize: \(.*\)/\1/gp'`
   SEUsedNearlineSize=`cat ${fil} | sed -n '/^GlueSEUsedNearlineSize:/ s/GlueSEUsedNearlineSize: \(.*\)/\1/gp'`
   SESite=`cat ${fil} | sed -n '/^GlueForeignKey: GlueSiteUniqueID=/ s/GlueForeignKey: GlueSiteUniqueID=\(.*\)/\1/gp'`
   echo "GlueSiteUniqueID: ${SESite}"
#SLName=`cat ${fil} | /^GlueForeignKey: GlueSLUniqueID=/ s/GlueForeignKey: GlueSLUniqueID==\(.*\)/\1/gp'`
   /bin/rm ${fil}
#
   for t in ${varp}; do
     echo "Glue${t}: "`eval echo "$"${t}`
   done
   echo ""
#
   for t in ${var}; do
     if test -z `eval echo "$"${t}`; then
       echo "Error: No Glue${t} attribute present"
       echo "Command executed: "
       echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueSE)(GlueSEUniqueID="$hst"))' Glue${t}"
       Stat=2
     fi
   done
fi
exit ${Stat}
#
#if test ${SLName}; then
#  SEID=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueSL)(GlueSLUniqueID='$hst'))' GlueForeignKey |  sed -n '/^GlueForeignKey: GlueSEUniqueID=/ s/^GlueForeignKey: GlueSEUniqueID=\(.*\)/\1/gp'`
#  if test ${SEID} -a ${SEID} = ${hst}; then
#     exit 0
#  else
#     echo "Error: No GlueSL object found for host ${hst}"
#     echo "Command executed: "
#     echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueSL)(GlueSLUniqueID="$hst"))'"
#     exit 2
#  fi
#else
#  exit 0
#fi
#

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
Stat=0
#
VOInfoTags=(`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID='$hst'))' GlueVOInfoTag | sed -n '/^GlueVOInfoTag: / s/^GlueVOInfoTag: //gp'`) 
#
if test ${#VOInfoTags[@]} -eq 0; then
   echo ""
   echo "Error: No VOInfoTags defined at this site"
   echo "Command executed: "
   echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID="$hst"))' GlueVOInfoTag"
   echo ""
   Stat=2
fi
#
VOInfos=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID='$hst'))' GlueVOInfoName | sed -n '/^GlueVOInfoName: / s/^GlueVOInfoName: //gp' | sort -u`
if test -z "${VOInfos}"; then
  echo ""
  echo "Error: No GlueVOInfo objects defined for this SE"
  echo "Command executed: "
  echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID="$hst"))' GlueVOInfoName"
  echo ""
  Stat=2
fi
if test ! -z "${VOInfos}"; then
#
  for void in `echo $VOInfos`; do
    fil=/tmp/${void}$$
    ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID='$hst')(GlueVOInfoName='$void'))' GlueVOInfoPath GlueVOInfoAccessControlBaseRule GlueChunkKey > ${fil}
    echo ""
    cat ${fil}
    var="VOInfoPath"
    VOInfoPath=(`cat ${fil} | sed -n '/^GlueVOInfoPath:/ s/GlueVOInfoPath: //gp'`)
    if test ${#VOInfoPath[@]} -gt 1; then
       echo ""
       echo "Error: Multiple VOInfoPath defined for VOInfoName=$voids"
       echo "Command executed: "
       echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID="$hst")(GlueVOInfoName="$void"))' GlueVOInfoPath"
       echo ""
       Stat=2
     fi 
     VOInfoAccessControlBaseRule=(`cat ${fil} | sed -n '/^GlueVOInfoAccessControlBaseRule:/ s/GlueVOInfoAccessControlBaseRule: //gp'`)
     VOInfoSALocalID=(`cat ${fil} | sed -n '/^GlueChunkKey: GlueSALocalID=/ s/GlueChunkKey: GlueSALocalID=//gp'`)
    if test ${#VOInfoSALocalID[@]} -gt 1; then
       echo ""
       echo "Error: Multiple VOInfoSALocalID defined for VOInfoName=$voids"
       echo "Command executed: "
       echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueVOInfo)(GlueVOInfoName="$void")(GlueChunkKey=GlueSALocalID=*))'"
       echo ""
       Stat=2
     fi
     /bin/rm ${fil}
     for t in ${var}; do
         echo "Glue${t}: "`eval echo "$"${t}`
     done
     echo ""
#
     for t in ${var}; do
         if test -z `eval echo "$"${t}`; then
            echo ""
            echo "Error: No Glue${t} attribute present for GlueVOInfo with GlueVOInfoName=${void}"
            echo "Command executed: "
            echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueVO)(GlueChunkKey=GlueSEUniqueID="$hst")(GlueVOInfoName="$void"))' Glue${t}"
            echo ""
            Stat=2
         fi
     done
     fil=/tmp/${void}$$
     ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID='$hst')(GlueSALocalID='${VOInfoSALocalID[0]}'))' GlueSAAccessControlBaseRule | sed -n '/^GlueSAAccessControlBaseRule: / s/^GlueSAAccessControlBaseRule: //gp' > ${fil}
     for i in `seq 1 ${#VOInfoAccessControlBaseRule[@]}`; do
         grep ${VOInfoAccessControlBaseRule[$i-1]} ${fil} >/dev/null 2>&1
         if test $? -ne 0; then
            echo ""
            echo "Error: GlueSA associated to VOInfoName=${void} does not match GlueVOInfoAccessControlBaseRule=${VOInfoAccessControlBaseRule[0]}"
            echo "Command executed: "
            echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID="$hst")(GlueSALocalID="${VOInfoSALocalID[0]}"))' GlueSAAccessControlBaseRule"
            echo ""
            Stat=2
         fi
     done
     /bin/rm ${fil}
  done
fi
#
exit ${Stat}
#

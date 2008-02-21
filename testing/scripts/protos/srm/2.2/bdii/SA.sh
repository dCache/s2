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
VOs=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID='$hst'))' GlueSAAccessControlBaseRule | sed -n '/^GlueSAAccessControlBaseRule: / s/^GlueSAAccessControlBaseRule: [VO:|VOMS:]*//gp' | sort -u`
if test -z "${VOs}"; then
  echo ""
  echo "Error: No GlueSA object defined or GlueSAAccessControlBaseRule attribute missing"
  echo "Command executed: "
  echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID="$hst"))' GlueSAAccessControlBaseRule"
  echo ""
  Stat=2
else
  ops=`echo $VOs | grep -c ops`
  if test ${ops} -lt 1; then
    echo ""
    echo "Error: No GlueSA object for VO ops defined"
    echo ""
    Stat=2
  fi
  ops=`echo $VOs | grep -c dteam`
  if test ${ops} -lt 1; then
    echo "Error: No GlueSA object for VO dteam defined"
    echo ""
    Stat=2
  fi
fi
#
# Check if this service has only an SRM v2.2 endpoint. In this case
# the old GlueSA with GlueSAID=<vo> is not needed.
#
srm11=1
ServType=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueService)(GlueServiceUniqueID=httpg://'$hst'*/srm/managerv1)(GlueServiceVersion=1.1.0))' GlueServiceType |  sed -n '/^GlueServiceType: / s/GlueServiceType: //pg'`
if test -z "${ServType}"; then
  echo "Warning: This host does not run an SRM v1.1 service"
  echo ""
  srm11=0
fi
#
for vo in `echo $VOs`; do
  if test ${srm11} -gt 0; then 
    fil=/tmp/${vo}$$
    ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID='$hst')(|(GlueSAAccessControlBaseRule='${vo}')(GlueSAAccessControlBaseRule=VO:'${vo}'))(GlueSALocalID='${vo}'))' GlueSAPath GlueSAPolicyFileLifeTime GlueSAStateAvailableSpace > ${fil}
    if test ! -s ${fil}; then
       echo ""
       echo "Error: No GlueSA object for VO ${vo} defined with GlueSALocalID=${vo}"
       echo ""
       /bin/rm ${fil}
       Stat=2
    else 
       echo ""
       cat ${fil}
       var="SAPath SAPolicyFileLifeTime SAStateAvailableSpace"
       SAPath=`cat ${fil} | sed -n '/^GlueSAPath:/ s/GlueSAPath: //gp'`
       SAPolicyFileLifeTime=`cat ${fil} | sed -n '/^GlueSAPolicyFileLifeTime:/ s/GlueSAPolicyFileLifeTime: //gp'`
       if test "x${SAPolicyFileLifeTime}" != "xpermanent"; then
         echo "Error: GlueSAPolicyFileLifeTime value different than permanent for GlueSA with GlueSALocalID=${vo}"
         Stat=2
         echo ""
       fi
       SAStateAvailableSpace=`cat ${fil} | sed -n '/^GlueSAStateAvailableSpace:/ s/GlueSAStateAvailableSpace: //gp'`
       /bin/rm ${fil}
       for t in ${var}; do
         echo "Glue${t}: "`eval echo "$"${t}`
       done
       echo ""
#
       for t in ${var}; do
         if test -z `eval echo "$"${t}`; then
            echo ""
            echo "Error: No Glue${t} attribute present for GlueSA with GlueSALocalID=${vo}"
            echo "Command executed: "
            echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID="$hst")(GlueSAAccessControlBaseRule=*"$vo")(GlueSALocalID="${vo}"))' Glue${t}"
            echo ""
            Stat=2
         fi
       done
    fi
  fi
#
  SAIDs=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID='$hst')(|(GlueSAAccessControlBaseRule='${vo}')(GlueSAAccessControlBaseRule=VO:'${vo}'))(!(GlueSALocalID='${vo}')))' GlueSALocalID | sed -n '/^GlueSALocalID: / s/^GlueSALocalID: //gp'`
#
#  SAIDs=`echo ${SAID} | sed 's/'${vo}'//gp'`
  if test "x${SAIDs}" = "x" ; then
     echo ""
     echo "Error: No GlueSA object for VO ${vo} defined with GlueSALocalID!=${vo}"
     echo ""
     Stat=2
  fi
# 
  for sa in `echo ${SAIDs}`; do
    safil=/tmp/SA$$
    ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID='$hst')(GlueSAAccessControlBaseRule=*'${vo}')(GlueSALocalID='${sa}'))' GlueSAPath GlueSAPolicyFileLifeTime GlueSAStateAvailableSpace GlueSARetentionPolicy GlueSAAccessLatency GlueSATotalOnlineSize GlueSAUsedOnlineSize GlueSATotalNearlineSize GlueSAUsedNearlineSize > ${safil}
    if test ! -s ${safil}; then
     echo ""
     echo "Error: No GlueSA object for VO ${vo} defined with GlueSALocalID=${sa}"
     echo ""
     /bin/rm ${safil}
     Stat=2
    else
      echo ""
      cat ${safil}
      varp="SAPath SAPolicyFileLifeTime SAStateAvailableSpace SARetentionPolicy SAAccessLatency SATotalOnlineSize SATotalNearlineSize"
      var="SAPath SAPolicyFileLifeTime SAStateAvailableSpace SARetentionPolicy SAAccessLatency"
      SAPath=`cat ${safil} | sed -n '/^GlueSAPath: / s/GlueSAPath: //gp'`
      SAPolicyFileLifeTime=`cat ${safil} | sed -n '/^GlueSAPolicyFileLifeTime: / s/GlueSAPolicyFileLifeTime: //gp'`
      SAStateAvailableSpace=`cat ${safil} | sed -n '/^GlueSAStateAvailableSpace: / s/GlueSAStateAvailableSpace: //gp'`
      SARetentionPolicy=`cat ${safil} | sed -n '/^GlueSARetentionPolicy: / s/GlueSARetentionPolicy: //gp'`
      SAAccessLatency=`cat ${safil} | sed -n '/^GlueSAAccessLatency: / s/GlueSAAccessLatency: //gp'`
      SATotalOnlineSize=`cat ${safil} | sed -n '/^GlueSATotalOnlineSize: / s/GlueSATotalOnlineSize: //gp'` 
      SATotalNearlineSize=`cat ${safil} | sed -n '/^GlueSASATotalNearlineSize: / s/GlueSATotalNearlineSize: //gp'`
      /bin/rm ${safil}
      for t in ${varp}; do
        echo "Glue${t}: "`eval echo "$"${t}`
      done
      echo ""
      for t in ${var}; do
        if test -z `eval echo "$"${t}`; then
         echo ""
         echo "Error: No Glue${t} attribute present"
         echo "Command executed: "
         echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID="$hst")(GlueSAAccessControlBaseRule=*"$vo")(GlueSALocalID="${sa}"))' Glue${t}"
         echo ""
         Stat=2
        fi
      done
    fi
  done
done
#
# Now processing the VOInfo objects
#
VOInfo=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID='$hst'))' GlueChunkKey | sed -n '/^GlueChunkKey: GlueSALocalID=/ s/^GlueChunkKey: GlueSALocalID=//gp' | sort -u`
#
if test -z "$VOInfo" ; then
  echo ""
  echo "Error: No GlueVOInfo objects defined for any VO"
  echo "Command executed: "
  echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID="$hst"))'"
  echo ""
  Stat=2
else
  oldvo=""
  for votag in `echo $VOInfo`; do
    SAID=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID='$hst')(GlueSALocalID='${votag}'))' GlueSALocalID | sed -n '/^GlueSALocalID: / s/^GlueSALocalID: //gp' | sort -u`
    if test -z "$SAID" ; then
      echo ""
      echo "Error: No GlueSA object found for ${votag}"
      echo "Command executed: "
      echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueSA)(GlueChunkKey=GlueSEUniqueID="$hst")(GlueSALocalID="${votag}"))'"
      echo ""
      Stat=2
    else
      vo=`echo ${votag} | cut -d: -f1`
      if test "x${vo}" != "x${oldvo}"; then
        oldvo=${vo}
        VOInfoTags=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID='$hst')(|(GlueVOInfoAccessControlBaseRule='${vo}')(GlueVOInfoAccessControlBaseRule=VO:'${vo}')(GlueVOInfoAccessControlBaseRule=VOMS:/'${vo}'*)))' GlueVOInfoTag | sed -n -e '/^GlueVOInfoTag: / s/GlueVOInfoTag: //pg'`
        if test -z "${VOInfoTags}"; then
          echo ""
          echo "Warning: No GlueVOInfo object class or GlueChunkKey/GlueVOInfoAccessControlBaseRule attribute missing"
          echo "Command executed: "
          echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID="$hst")(|(GlueVOInfoAccessControlBaseRule="$vo")(GlueVOInfoAccessControlBaseRule=VO:"$vo")(GlueVOInfoAccessControlBaseRule=VOMS:/"${vo}"*)))'"
          echo ""
        else
          for tag in `echo $VOInfoTags`; do
            voInfofil=/tmp/voInfo$$
            ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID='$hst')(|(GlueVOInfoAccessControlBaseRule='${vo}')(GlueVOInfoAccessControlBaseRule=VO:'${vo}')(GlueVOInfoAccessControlBaseRule=VOMS:/'${vo}'*))(GlueVOInfoTag='${tag}'))' GlueVOInfoLocalID GlueVOInfoName GlueVOInfoPath GlueVOInfoTag > ${voInfofil}
            if test ! -s ${voInfofil}; then
               echo ""
               echo "Error: No GlueVOInfo object returned by query"
               echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID="$hst"')(|(GlueVOInfoAccessControlBaseRule="${vo}")(GlueVOInfoAccessControlBaseRule=VO:"${vo}")(GlueVOInfoAccessControlBaseRule=VOMS:/"${vo}"*))(GlueVOInfoTag="${tag}"))'"
               echo ""
               /bin/rm -f ${voInfofil}
               Stat=2
            else
               echo ""
               cat ${voInfofil}
               var="VOInfoLocalID VOInfoName VOInfoPath  VOInfoTag"
               VOInfoLocalID=`cat ${voInfofil} | sed -n '/^GlueVOInfoLocalID: / s/GlueVOInfoLocalID: //gp'`
               VOInfoName=`cat ${voInfofil} | sed -n '/^GlueVOInfoName: / s/GlueVOInfoName: //gp'`
               VOInfoPath=`cat ${voInfofil} | sed -n '/^GlueVOInfoPath: / s/GlueVOInfoPath: //gp'`
               VOInfoTag=`cat ${voInfofil} | sed -n -e '/^GlueVOInfoTag: / s/GlueVOInfoTag: //pg'`
               /bin/rm ${voInfofil}
               for t in ${var}; do
                  echo "Glue${t}: "`eval echo "$"${t}`
               done
#
               for t in ${var}; do
                  if test -z `eval echo "$"${t}`; then
                     echo ""
                     echo "Error: No Glue${t} attribute present"
                     echo "Command executed: "
                     echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueVOInfo)(GlueChunkKey=GlueSEUniqueID="$hst")(GlueSAAccessControlBaseRule=*"$vo"))' Glue${t}"
                     echo ""
                     Stat=2
                  fi
               done
            fi
          done
        fi
      fi
    fi
  done
fi
exit ${Stat}
#

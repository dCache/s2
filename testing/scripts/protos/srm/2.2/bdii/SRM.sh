#!/bin/sh
if test $# -le 0; then
  echo "Usage: $0 <srm v2.2 endpoint host>"
  echo "Please provide srm v2.2 endpoint host name"
  exit 1
else
   hst=$1
fi
if test -z ${LCG_GFAL_INFOSYS}; then
  echo "Error: environmental variable LCG_GFAL_INFOSYS unset"
  exit 1
fi
Stat=0
ServType=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueService)(GlueServiceUniqueID=httpg://'$hst'*/srm/managerv2)(GlueServiceVersion=2.2.0))' GlueServiceType |  sed -n '/^GlueServiceType: / s/GlueServiceType: //pg'`
if test -z "${ServType}"; then
  echo "Error: No GlueService object defined or wrong GlueServiceUniqueID or wrong GlueServiceVersion"
  echo "Command executed: "
  echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueService)(GlueServiceUniqueID=httpg://"$hst"\*/srm/managerv2)(GlueServiceVersion=2.2.0))'"
  Stat=2
else
  if test "x${ServType}" != "xSRM"; then
    echo "Error: Incorrect GlueServiceType published for SRM v2.2 service"
    echo "Command executed: "
    echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueService)(GlueServiceUniqueID=httpg://"$hst"\*/srm/managerv2)(GlueServiceVersion=2.2.0))' GlueServiceType"
    Stat=2
  fi
  echo "GlueServiceType: ${ServType}"
fi
#
ServVOs=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueService)(GlueServiceUniqueID=httpg://'$hst'*/srm/managerv2)(GlueServiceVersion=2.2.0))' GlueServiceAccessControlRule |  sed -n '/^GlueServiceAccessControlRule: / s/GlueServiceAccessControlRule: [VO:]*//pg'`
ops=`echo ${ServVOs} | grep -c ops`
if test ${ops} -lt 1; then
  echo "Error: No GlueService object for VO ops defined"
  Stat=2
else
  echo "VO ops served by SE $hst"
fi
ops=`echo ${ServVOs} | grep -c dteam`
if test ${ops} -lt 1; then
  echo "Error: No GlueService object for VO dteam defined"
  Stat=2
else
  echo "VO dteam served by SE $hst"
fi
#
exit ${Stat}
#

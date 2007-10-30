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
Prots=`ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueSEAccessProtocol)(GlueChunkKey=GlueSEUniqueID='$hst'))' GlueSEAccessProtocolLocalID | sed -n '/^GlueSEAccessProtocolLocalID: / s/^GlueSEAccessProtocolLocalID: //gp'`
if test -z "${Prots}"; then
  echo "Error: No GlueSEAccessProtocol objects defined"
  echo "Command executed: "
  echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueSEAccessProtocol)(GlueChunkKey=GlueSEUniqueID="$hst"))' GlueSEAccessProtocolLocalID"
  exit 2
else
  echo "The protocols supported are :"
  echo "${Prots}"
fi
exit 0
#

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
ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueCESEBindGroup)(GlueCESEBindGroupSEUniqueID='$hst'))' GlueCESEBindGroupCEUniqueID | sed -e '/^GlueCESEBindGroupCEUniqueID:/ N; s/\n //' | sed -n -e '/^GlueCESEBindGroupCEUniqueID:/ s/^GlueCESEBindGroupCEUniqueID: //pg' > /tmp/$$_Group
sort -u /tmp/$$_Group -o /tmp/$$_Group_sorted
rm /tmp/$$_Group
#
ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid" '(&(objectClass=GlueCESEBind)(GlueCESEBindSEUniqueID='$hst'))' GlueCESEBindCEUniqueID | sed '/^GlueCESEBindCEUniqueID:/ N; s/\n //' | sed -n -e '/^GlueCESEBindCEUniqueID:/ s/^GlueCESEBindCEUniqueID: //pg' > /tmp/$$_CE
sort -u /tmp/$$_CE -o /tmp/$$_CE_sorted
rm /tmp/$$_CE
#
if test -s /tmp/$$_Group_sorted -a -s /tmp/$$_Group_sorted ; then
  cmp -s /tmp/$$_Group_sorted /tmp/$$_CE_sorted
  excode=$?
  if test ${excode} -ne 0; then
    echo "Error: Inconsistencies between GlueCESEBind and GlueCESEBindGroup objects found"
    echo "Different results obtained using the following 2 queries:"
    echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueCESEBindGroup)(GlueCESEBindGroupSEUniqueID="$hst"))' GlueCESEBindGroupCEUniqueID"
    echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueCESEBind)(GlueCESEBindSEUniqueID="$hst"))' GlueCESEBindCEUniqueID"
    echo "The difference concerns the following CEs:"
    diff /tmp/$$_Group_sorted /tmp/$$_CE_sorted
    excode=2
  else
    echo "Success: The SE is close to the following CEs"
    cat /tmp/$$_Group_sorted
  fi
else
  echo "Error: No GlueCESEBind or GlueCESEBindGroup object found for ${hst}"
  echo "Commands executed: "
  echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueCESEBindGroup)(GlueCESEBindGroupSEUniqueID="$hst"))'"
  echo 'ldapsearch -LLL -h $LCG_GFAL_INFOSYS -x -b "o=grid"' "'(&(objectClass=GlueCESEBind)(GlueCESEBindSEUniqueID="$hst"))'"
  excode=2
fi
rm /tmp/$$_Group_sorted /tmp/$$_CE_sorted
exit ${excode}
#

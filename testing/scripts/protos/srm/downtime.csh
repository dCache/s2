#!/bin/csh
#
# Source the environment
#
#
switch ( "$1" )
  case "y"
     set down = 1
     breaksw
  case "n"
     set down = 0
     breaksw
  default:
     echo "Downtime option not recognized"
     exit 1
     breaksw
endsw

switch ( "$2" )
  case "castor":
     set tag = "22CASTORCERN"
     breaksw
  case "dcache":
     set tag = "22DCACHEFNAL"
     breaksw
  case "dpm":
     set tag = "22DPMCERN"
     breaksw
  case "drm":
     set tag = "22DRMLBNL"
     breaksw
  case "storm":
     set tag = "22STORM"
     breaksw
  default:
     echo "SRM implementation name not recognized"
     exit 1
     breaksw
endsw

set dirlis = ("avail" "basic" "usecase" "cross")
set logdir = "s2_logs"
set srcdir = "/home/tests/testing/scripts/protos/srm/2.2"
set downfil = "scheduled-downtime"
#
cd ${srcdir}
#
foreach tst ($dirlis)
   if ( "x${down}" == "x1" )  then
      echo "Scheduled downtime" > ${srcdir}/${tst}/${logdir}/${tag}/${downfil}
   else
   /bin/rm -f ${srcdir}/${tst}/${logdir}/${tag}/${downfil}
   endif
end
#
# That's all folks!
#

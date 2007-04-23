#!/bin/csh
#
# Source the environment
#
#
switch ( "$1" )
  case "basic":
     set srcdir = "/home/tests/testing/scripts/protos/srm/2.2/basic"
     set destdir = "/home/tests/devel/s2/testing/scripts/protos/srm/2.2/basic"
     breaksw
  case "cross":
     set srcdir = "/home/tests/testing/scripts/protos/srm/2.2/cross"
     set destdir = "/home/tests/devel/s2/testing/scripts/protos/srm/2.2/cross"
     breaksw
  case "usecase":
     set srcdir = "/home/tests/testing/scripts/protos/srm/2.2/usecase"
     set destdir = "/home/tests/devel/s2/testing/scripts/protos/srm/2.2/usecase"
     breaksw
  case "exhaust":
     set srcdir = "/home/tests/testing/scripts/protos/srm/2.2/exhaust"
     set destdir = "/home/tests/devel/s2/testing/scripts/protos/srm/2.2/exhaust"
     breaksw
  case "avail":
     set srcdir = "/home/tests/testing/scripts/protos/srm/2.2/avail"
     set destdir = "/home/tests/devel/s2/testing/scripts/protos/srm/2.2/avail"
     breaksw
  case "stress":
     set srcdir = "/home/tests/testing/scripts/protos/srm/2.2/stress"
     set destdir = "/home/tests/devel/s2/testing/scripts/protos/srm/2.2/stress"
     breaksw
  default:
     echo "Test directory option not recognized"
     exit 1
     breaksw
endsw
#
#
cd ${srcdir}
#
# Copy file
#
foreach s21 (`ls -C1 *.s2`)
   set s2 = `echo $s21 |cut -d_ -f2`
   if ( "x${s2}" == "x" )  then
     set s2 = $s21
   endif
   if ( ! -d ${destdir} ) echo "Creating directory ${destdir}"
   if ( ! -d ${destdir} ) mkdir -p ${destdir}
   echo "Copying file ${srcdir}/${s21} in file ${destdir}/${s2}"
   /bin/cp ${srcdir}/${s21} ${destdir}/${s2}
end
#
# Copy extra file *.env, make_result_web_page.sh and Makefile
#
if ( -d ${srcdir}/webmap ) /bin/cp ${srcdir}/webmap/* ${destdir}/webmap
/bin/cp ${srcdir}/s2.env ${destdir}
/bin/cp ${srcdir}/make_result_web_page.sh ${destdir}
/bin/cp ${srcdir}/Makefile ${destdir}
#
#
#
/bin/cp ${srcdir}/../../s2.env ${destdir}/../..
/bin/cp ${srcdir}/../../cycle.csh ${destdir}/../..
/bin/cp ${srcdir}/../../cycle_stress.csh ${destdir}/../..
/bin/cp ${srcdir}/../../web_config.conf ${destdir}/../..
/bin/cp ${srcdir}/../../env.csh ${destdir}/../..
/bin/cp ${srcdir}/../../env.sh ${destdir}/../..
/bin/cp ${srcdir}/../../make_backup.csh ${destdir}/../..
/bin/cp ${srcdir}/../../downtime.csh ${destdir}/../..
#
# That's all folks!
#

#!/bin/csh
#
# Source the environment
#
setenv GLOBUS_LOCATION /opt/globus
setenv GPT_LOCATION /opt/gpt
setenv PATH "/home/tests/s2/bin:$GLOBUS_LOCATION/bin:/usr/local/bin:$PATH"
setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/certificates
setenv S2_MULTI_SITE yes
rehash
#
switch ( "$1" )
  case "basic":
     set rootdir = "/home/tests/testing/scripts/protos/srm/2.2/basic"
     breaksw
  case "cross":
     set rootdir = "/home/tests/testing/scripts/protos/srm/2.2/cross"
     breaksw
  case "usecase":
     set rootdir = "/home/tests/testing/scripts/protos/srm/2.2/usecase"
     breaksw
  case "exhaust":
     set rootdir = "/home/tests/testing/scripts/protos/srm/2.2/exhaust"
     breaksw
  case "avail":
     set rootdir = "/home/tests/testing/scripts/protos/srm/2.2/avail"
     breaksw
  case "bdii":
     set rootdir = "/home/tests/testing/scripts/protos/srm/2.2/bdii"
     breaksw
  case "lcg-utils":
     set rootdir = "/home/tests/testing/scripts/protos/srm/2.2/lcg-utils"
     breaksw
  default:
     echo "Test directory option not recognized"
     exit 1
     breaksw
endsw
#
# Create new index file
#
cd ${rootdir}
/bin/rm -fr ${rootdir}/s2_logs/index.html
${rootdir}/make_result_web_page.sh ${rootdir}/s2_logs/index.html_new
#
# Create history directory
#
set datdir = ${rootdir}/history/`date +"%F"`
if ( ! -d ${datdir} ) mkdir -p ${datdir}
#
# Copy index file there
#
set tim = `date +"%k:%M"`
mkdir -p ${datdir}/${tim}
/bin/cp ${rootdir}/s2_logs/index.html_new ${datdir}/${tim}/index.html
/bin/mv ${rootdir}/s2_logs/index.html_new ${rootdir}/s2_logs/index.html
#
#  Copy history files
#
set list = "22CASTORCERN 22CASTORDEV 22DPMCERN 22STORM 22DRMLBNL 22DCACHEFNAL"
foreach dir (`echo $list`)
 if ( -d ${rootdir}/s2_logs/${dir} ) then
    mkdir -p ${datdir}/${tim}/${dir}
    /bin/cp ${rootdir}/s2_logs/${dir}/* ${datdir}/${tim}/${dir}
 endif 
end
#
# That's all folks!
#

#!/bin/csh
#
# Source the environment
#
setenv GLOBUS_LOCATION /opt/globus
setenv GPT_LOCATION /opt/gpt
setenv PATH "/home/flavia/s2/usr/bin:$GLOBUS_LOCATION/bin:/usr/local/bin:$PATH"
setenv X509_CERT_DIR /afs/cern.ch/project/gd/LCG-share/certificates
setenv S2_MULTI_SITE yes
rehash
#
# Set Root dir
#
if ( $# <= 0 ) then
 echo "Please specify test directory"
 echo "Possible options are: basic and cross" 
 exit 1
endif
#
switch ( "$1" )
  case "basic":
     set rootdir = "/home/flavia/testing/scripts/protos/srm/2.2/basic"
     breaksw
  case "cross":
     set rootdir = "/home/flavia/testing/scripts/protos/srm/2.2/cross"
     breaksw
  default:
     echo "Test directory option not recognized"
     exit 1 
     breaksw
endsw
#
#set list = "22DPMCERN 22STORM 22DRMLBNL 22CASTORCERN 22CASTORRAL 22DCACHEFNAL 22SRMVU"
set list = "22CASTORCERN 22DPMCERN 22STORM 22DRMLBNL 22DCACHEFNAL"
foreach impl (`echo $list`)
   onintr again
   unsetenv S2_TEST_SITE
   unsetenv S2_LOGS_DIR
   setenv S2_TEST_SITE $impl
   setenv S2_LOGS_DIR "./s2_logs/${S2_TEST_SITE}"
   cd ${rootdir}
   echo "Now processing $impl"
   make -e test
   echo "Done."
again: 
end
#
# Create new index file
#
cd ${rootdir}
#/bin/rm -fr ${rootdir}/s2_logs/index.html
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
foreach dir (`echo $list`)
 if ( -d ${rootdir}/s2_logs/${dir} ) then
    mkdir -p ${datdir}/${tim}/${dir}
    /bin/cp ${rootdir}/s2_logs/${dir}/* ${datdir}/${tim}/${dir}
 endif 
end
#
# That's all folks!
#

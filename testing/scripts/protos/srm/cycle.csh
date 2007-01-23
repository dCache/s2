#!/bin/tcsh
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
set srm_root = "/home/flavia/testing/scripts/protos/srm"
set typ = "$1"
if ( $# <= 0 ) then
 echo "Please specify test directory"
 echo "Possible options are: avail,basic,usecase and cross" 
 exit 1
endif
#
switch ( "${typ}" )
  case "basic":
     set rootdir = "${srm_root}/2.2/basic"
     set sleeptim = "30m"
     breaksw
  case "cross":
     set rootdir = "${srm_root}/2.2/cross"
     set sleeptim = "30m"
     breaksw
  case "exhaust":
     set rootdir = "${srm_root}/2.2/exhaust"
     set sleeptim = "10m"
     breaksw
  case "usecase":
     set rootdir = "${srm_root}/2.2/usecase"
     set sleeptim = "3m"
     breaksw
  case "avail":
     set rootdir = "${srm_root}/2.2/avail"
     set sleeptim = "1m"
     breaksw
  default:
     echo "Test directory option not recognized"
     exit 1 
     breaksw
endsw
#
# Do not run if another instance is running
#
echo "Wait till other instances of cycle are done"
set i = "0"
set ssleeptim = "3m"
while ( "$i" == "0" )
   @ outp = `/bin/ps -C cycle.csh -o pid,cmd | grep ${typ} | grep -c -v $$`
   if ( ${outp} >= 2 ) then
      echo "Too many instances of cycle.csh "$1" running. Exiting ..."
      exit 0
   endif
   if ( ${outp} == 1 ) then
      echo "Waiting ${ssleeptim} ..."
      sleep ${ssleeptim}
      set i = "0"
   else
      set i = "1"
   endif
end
#
#set list = "22CASTORCERN 22CASTORDEV 22CASTORRAL 22DCACHEFNAL 22DPMCERN 22DRMLBNL 22SRMVU 22STORM"
set list = "22CASTORCERN 22CASTORDEV 22DCACHEFNAL 22DPMCERN 22DRMLBNL 22STORM"
foreach impl (`echo $list`)
   onintr again
   unsetenv S2_TEST_SITE
   unsetenv S2_LOGS_DIR
   setenv S2_TEST_SITE $impl
   setenv S2_LOGS_DIR "${rootdir}/s2_logs/${S2_TEST_SITE}"
   cd ${rootdir}
   echo "Now processing $impl `date +'%F %k:%M'`"
   if ( ! -d /home/flavia/logs/${S2_TEST_SITE} ) then
      mkdir -p /home/flavia/logs/${S2_TEST_SITE}
   endif
   make -e test >& /home/flavia/logs/${S2_TEST_SITE}/$1-`date +"%F-%k:%M"`.log &
   echo "Done."
again: 
end
#
#  Wait for all jobs to finish
#
echo "Wait till all tests are done"
set i = "0"
while ( "$i" == "0" )
   set outp = `ps -C make -o ppid,cmd | grep "make" | grep "$$"` 
   if ( "x${outp}" != "x" ) then
      echo "Waiting ${sleeptim} ..."
      sleep ${sleeptim}
      set i = "0"
   else
      set i = "1"
   endif
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
#
#
${srm_root}/make_tars.csh $typ
# That's all folks!
#

#!/bin/tcsh
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
# Set Root and lock dir
#
set srm_root = "/home/tests/testing/scripts/protos/srm"
set lock_dir = "/home/tests/lock"
set logs_dir = "/home/tests/logs"
if ( ! -e ${lock_dir} ) then
   mkdir -p ${lock_dir}
endif
#
#if ( $# <= 0 ) then
# echo "Please specify test directory"
# echo "Possible options are: avail,basic,usecase,cross,stress" 
# exit 1
#endif
#
set typ = "stress"
#
set rootdir = "${srm_root}/2.2/stress"
set sleeptim = "2m"
#
# Create lock file
#
set lock_fil = ${lock_dir}/${typ}-$$
touch ${lock_fil}
#
# Do not run if another instance is running
#
echo "Wait till other instances of cycle are done"
set i = "0"
set ssleeptim = "3m"
set outfp = ""
while ( "$i" == "0" )
#   @ outp = `/bin/ps -C cycle.csh -o pid,cmd | grep ${typ} | grep -c -v $$`
   @ outp = `/bin/ls -C1 ${lock_dir}/${typ}* | grep -c -v "${lock_fil}"`
   set outf = `/bin/ls -C1 ${lock_dir}/${typ}* | grep -v "${lock_fil}"`
   if ( ${outp} > 1 ) then
      echo "Too many instances of cycle_stress.csh running. Exiting ..."
      /bin/rm ${lock_fil}
      exit 0
   endif
   if ( ${outp} == 1 ) then
      if ( "x${outfp}" == x ) then
         set outfp = ${outf}
      endif
      if ( ${outf} == ${outfp) ) then
         echo "Waiting ${ssleeptim} ..."
         sleep ${ssleeptim}
         set i = "0"
      else
         set i = "1"
      endif
   else
      set i = "1"
   endif
end
#
#set list = "22CASTORSTRESS 22DCACHESTRESS 22DCACHEDESY 22DPMCERN 22DRMLBNL 22SRMVU 22STORM"
set list = "22DCACHESTRESS 22DCACHEDESY 22DRMLBNL 22STORM"
#
set f_logs = ""
foreach impl (`echo $list`)
   onintr again
   unsetenv S2_TEST_SITE
   unsetenv S2_LOGS_DIR
   setenv S2_TEST_SITE $impl
   setenv S2_LOGS_DIR "${rootdir}/s2_logs/${S2_TEST_SITE}"
   cd ${rootdir}
   echo "Now processing $impl `date +'%F %k:%M'`"
   if ( ! -d ${logs_dir}/${S2_TEST_SITE} ) then
      mkdir -p ${logs_dir}/${S2_TEST_SITE}
   endif
   set f_log = ${logs_dir}/${S2_TEST_SITE}/stress-`date +"%F-%R"`.log
   set f_logs = ( ${f_logs} ${f_log} ) 
   make -e test >& ${f_log}
   echo "Done."
again: 
end
#
#  Wait for all jobs to finish
#
echo "Wait till all tests are done"
set i = "0"
while ( "$i" == "0" )
   set outp = `/sbin/fuser $f_logs`
#   set outp = `ps -C make -o ppid,cmd | grep "make" | grep "$$"` 
   if ( "x${outp}" != "x" ) then
      echo "Waiting ${sleeptim} ..."
      sleep ${sleeptim}
      set i = "0"
   else
      set i = "1"
   endif
end
#
# Clean-up unneeded directories
#
foreach dirl ( `/bin/ls -C1 ${rootdir}/s2_logs | grep -v index.html` )
   set dmatch = `echo "${list}" | grep -c ${dirl}`
   if ( ${dmatch} == 0 ) then
     /bin/rm -fr ${rootdir}/s2_logs/${dirl}
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
#set tim = `date +"%k:%M"`
set tim = `date +"%R"`
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
#  Create tars and copy them in AFS space
#
${srm_root}/make_tars.csh $typ
#
#  Remove lock file
#
/bin/rm ${lock_fil}
#
# That's all folks!
#

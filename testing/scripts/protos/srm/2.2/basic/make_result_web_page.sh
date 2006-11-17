#!/bin/bash
#
# Useful variables
#
S2_LOGS=./s2_logs
tim=`date +"%A %e %B %Y %I:%M%P %Z"`
if test $1; then
   Index_File=$1
else
   Index_File=${S2_LOGS}/index.html
fi
#
# Rename the index file for keeping history
#
#if test -e ${Index_File}; then
#   mv ${Index_File} ${Index_File}_old
#fi
#
# Start building index.html
#
cat << EOF >> ${Index_File}
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2//EN"> 
<HTML> 
<BODY BGCOLOR="#FFFFCC">
<HEAD> 
  <TITLE>Summary of S2 SRM v2.2 basic tests</TITLE> 
</HEAD> 
<CENTER>
<H3>Summary of S2 SRM v2.2 basic test - $tim</H3>
</CENTER>
<TABLE BORDER=2 CELLPADDING=1 CELLSPACING=1> 
<!-- header --> 
<TR BGCOLOR="#CCCCCC"> 
  <TH VALIGN=center ALIGN=center><FONT SIZE=3><B>SRM function</B></FONT></TH> 
  <TH VALIGN=top ALIGN=center>   </TH> 
EOF
for dir in `ls -1 $S2_LOGS | grep -v index.html`;
do
  case "${dir}" in
     22DPMCERN) sitetag="CERN<br>DPM"
     ;;
     22CASTORCERN) sitetag="CERN<br>CASTOR"
     ;;
     22CASTORRAL) sitetag="RAL<br>CASTOR"
     ;;
     22DCACHEFNAL) sitetag="FNAL<br>DCACHE"
     ;;
     22STORM) sitetag="STORM"
     ;;
     22DRMLBNL) sitetag="LBNL<br>DRM"
     ;;
     22SRMVU) sitetag="VU<br>SRM"
     ;;
     *) echo "Unrecognized endpoint - Exiting"; exit 1
     ;;
  esac
  if test -e ${S2_LOGS}/${dir}/Ping.log; then
     endp=`grep EndPoint ${S2_LOGS}/${dir}/Ping.log | cut -d= -f2`
     echo "<TH VALIGN=center ALIGN=center><FONT SIZE=3><A HREF=${endp}>$sitetag</A></FONT></TH>" >> ${Index_File}
  else
     echo "<TH VALIGN=top ALIGN=center><FONT SIZE=3><A HREF="Server down">$sitetag</A></FONT></TH>" >> ${Index_File}
  fi
done
echo "</TR>" >> ${Index_File}

for s21 in `ls -1 *.s2`;
do
   echo "<TR>" >> ${Index_File}
   s2_t=`basename ${s21} .s2`
   s2=`echo $s2_t |cut -d_ -f2` 
   if test -z "$s2" ; then
     s2="$s2_t"
   fi
   echo "<TD VALIGN=top ALIGN=center><B>${s2}</B></TD>" >> ${Index_File}
   echo "<TD VALIGN=top ALIGN=center>   </TD>" >> ${Index_File}
   for dir in `ls -1 $S2_LOGS | grep -v index.html`;
   do
    if test -e ${S2_LOGS}/${dir}/exit.log; then
      err=`grep \($s2\.sh\) ${S2_LOGS}/${dir}/exit.log | cut -d" " -f1`
      if test $err ; then
         if test $err -eq 0 ; then
            color="GREEN"
         else
            color="RED"
         fi
      else
         color="RED"
      fi
    fi
    echo "<TD BGCOLOR=${color} VALIGN=center ALIGN=center>" >> ${Index_File}
    if test -e ${S2_LOGS}/${dir}/${s2}.out; then
      echo "<A HREF=${dir}/${s2}.out>StdOut</A>" >> ${Index_File}
    fi
    if test -e ${S2_LOGS}/${dir}/${s2}.log; then
      echo " <A HREF=${dir}/${s2}.log>Log</A>" >> ${Index_File}
    fi
    echo "</TD>" >> ${Index_File}
    done
    echo "</TR>" >> ${Index_File}
done
cat << EOF1 >> ${Index_File}
</TABLE>
</BODY>
</HTML>
EOF1
#
# Append the old index file for keeping history
#
#if test -e ${Index_File}_old; then
#   cat ${Index_File}_old >> ${Index_File}
#fi
#

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
     22CASTORDEV) sitetag="CERN<br>CASTORDEV"
     ;;
     22CASTORRAL) sitetag="RAL<br>CASTOR"
     ;;
     22DCACHEFNAL) sitetag="FNAL<br>DCACHE"
     ;;
     22DCACHESTRESS) sitetag="FNAL<br>DCACHE"
     ;;
     22DCACHEDESY) sitetag="DESY<br>DCACHE"
     ;;
     22STORM) sitetag="STORM"
     ;;
     22DRMLBNL) sitetag="LBNL<br>BeStMan"
     ;;
     22SRMVU) sitetag="VU<br>SRM"
     ;;
     *) echo "Unrecognized endpoint - Exiting"; exit 1
     ;;
  esac
  if test -e ${S2_LOGS}/${dir}/scheduled-downtime; then
     echo "<TH VALIGN=top ALIGN=center><FONT SIZE=3>$sitetag<BR>Scheduled downtime</FONT></TH>" >> ${Index_File}
  else
     echo "<TH VALIGN=top ALIGN=center><FONT SIZE=3><A HREF=${dir}/index.html>$sitetag</A></FONT></TH>" >> ${Index_File}
  fi
done
echo "</TR>" >> ${Index_File}

echo "<TR><TH BGCOLOR=OLIVE COLSPAN=10 HEIGHT=40>" >> ${Index_File}
echo "WLCG MoU SRM v2.2 methods</TH></TR>" >> ${Index_File}

delim=1
for s21 in `ls -1 *.s2`;
do
   echo "<TR>" >> ${Index_File}
   s2_t=`basename ${s21} .s2`
   s2=`echo $s2_t |cut -d_ -f2` 
   if test -z "$s2" ; then
     s2="$s2_t"
   else
     s2_num=`echo $s2_t |cut -d_ -f1`
     if test $s2_num -ge 70 -a $delim -eq 2; then
       echo "<TH BGCOLOR=OLIVE COLSPAN=10 HEIGHT=40>" >> ${Index_File} 
       echo "WLCG non MoU SRM v2.2 methods</TH></TR><TR>" >> ${Index_File}
       delim=0
     fi
     if test $s2_num -ge 62 -a $delim -eq 1; then
       echo "<TH BGCOLOR=ORANGE COLSPAN=10 HEIGHT=40>" >> ${Index_File}
       echo "WLCG MoU SRM v2.2 methods needed by end of 2007</TH></TR><TR>" >> ${Index_File}
       delim=2
     fi
   fi
   echo "<TD VALIGN=top ALIGN=left><B>${s2}</B></TD>" >> ${Index_File}
   echo "<TD VALIGN=top ALIGN=left>   </TD>" >> ${Index_File}
   for dir in `ls -1 $S2_LOGS | grep -v index.html`;
   do
    if test -e ${S2_LOGS}/${dir}/exit.log; then
      err=`grep "\(${s2_t}\.sh\)" ${S2_LOGS}/${dir}/exit.log | cut -d" " -f1`
      if test "$err" ; then
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
    if test -e ${S2_LOGS}/${dir}/${s2_t}.out; then
      echo "<A HREF=${dir}/${s2_t}.out>StdOut</A>" >> ${Index_File}
    fi
    if test -e ${S2_LOGS}/${dir}/${s2_t}.log; then
      echo " <A HREF=${dir}/${s2_t}.log>Log</A>" >> ${Index_File}
    fi
    echo "</TD>" >> ${Index_File}
    done
    echo "</TR>" >> ${Index_File}
done
cat << EOF1 >> ${Index_File}
</TBODY>
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

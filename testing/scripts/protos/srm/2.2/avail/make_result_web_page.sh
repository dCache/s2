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
  <TITLE>Summary of S2 SRM v2.2 availability tests</TITLE> 
</HEAD> 
<CENTER>
<H3>Summary of S2 SRM v2.2 availability test - $tim</H3>
</CENTER>
<TABLE BORDER=2 CELLPADDING=1 CELLSPACING=1> 
<!-- header --> 
<TR BGCOLOR="#CCCCCC"> 
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

echo "<TR>" >> ${Index_File}

color="NULL"

for dir in `ls -1 $S2_LOGS | grep -v index.html`;
do
  for s2 in `ls -1 *.s2`;
  do
   s2_t=`basename ${s2} .s2`
   if test "$color" != "RED" ; then 
      if test -e ${S2_LOGS}/${dir}/exit.log; then
         err=`grep \(${s2_t}\.sh\) ${S2_LOGS}/${dir}/exit.log | cut -d" " -f1`
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
   fi
  done
if test "$color" = "GREEN"; then
   echo "<TD BGCOLOR=\""$color"\" VALIGN=center ALIGN=center><B>UP</B></TD>" >> ${Index_File}
else
   echo "<TD BGCOLOR=\"RED\" VALIGN=center ALIGN=center><B>DOWN</B></TD>" >> ${Index_File}
fi
color="NULL"
done

echo "</TD>" >> ${Index_File}
echo "</TR>" >> ${Index_File}
cat << EOF1 >> ${Index_File}
</TBODY>
</TABLE>
</BODY>
</HTML>
EOF1
#

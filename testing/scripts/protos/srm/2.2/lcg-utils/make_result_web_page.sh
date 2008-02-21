#!/bin/bash
#
# Useful variables
#
S2_LOGS=./s2_logs
WEB_CONF=../../web_config.conf
tim=`date +"%A %e %B %Y %I:%M%P %Z"`
if test $1; then
   Index_File=$1
else
   Index_File=${S2_LOGS}/index.html
fi
#
# Start building index.html
#
cat << EOF >> ${Index_File}
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2//EN"> 
<HTML> 
<BODY BGCOLOR="#FFFFCC">
<HEAD> 
  <TITLE>Summary of S2 SRM v2.2 lcg-utils tests</TITLE> 
</HEAD> 
<CENTER>
<H3>Summary of S2 SRM v2.2 lcg-utils test - $tim</H3>
</CENTER>
<TABLE BORDER=2 CELLPADDING=1 CELLSPACING=1> 
<!-- header --> 
<TR BGCOLOR="#CCCCCC"> 
  <TH VALIGN=center ALIGN=center><FONT SIZE=3><B>SRM test</B></FONT></TH> 
  <TH VALIGN=top ALIGN=center>   </TH> 
EOF
for dir in `ls -1 $S2_LOGS | grep -v index.html`;
do
  sitetag=`awk '/^'${dir}' / {print $2}' ${WEB_CONF}`
  if test "x${sitetag}" == "x"; then
     echo "Unrecognized endpoint - Exiting"; exit 1
  fi
  if test -e ${S2_LOGS}/${dir}/scheduled-downtime; then
     echo "<TH VALIGN=top ALIGN=center><FONT SIZE=3>$sitetag<BR>Scheduled downtime</FONT></TH>" >> ${Index_File}
  else
     echo "<TH VALIGN=top ALIGN=center><FONT SIZE=3><A HREF=${dir}/index.html>$sitetag</A></FONT></TH>" >> ${Index_File}
  fi
done

echo "</TR>" >> ${Index_File}

for s21 in `ls -1 *.sh`;
do
   echo "<TR>" >> ${Index_File}
   s2_t=`basename ${s21} .sh`
   if test "x${s2_t}" != "xmake_result_web_page"; then
     s2=`echo $s2_t |cut -d_ -f2` 
     if test -z "$s2" ; then
       s2="$s2_t"
     else
       s2_num=`echo $s2_t |cut -d_ -f1`
     fi
     echo "<TD VALIGN=top ALIGN=left><B><A HREF=../${s2}.sh>${s2}</A></B></TD>" >> ${Index_File}
     echo "<TD VALIGN=top ALIGN=left>   </TD>" >> ${Index_File}
     for dir in `ls -1 $S2_LOGS | grep -v index.html`;
     do
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
   fi
done
cat << EOF1 >> ${Index_File}
</TBODY>
</TABLE>
</BODY>
</HTML>
EOF1
#

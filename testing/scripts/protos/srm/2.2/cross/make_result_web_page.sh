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
  <TITLE>Summary of S2 SRM v2.2 cross tests</TITLE> 
</HEAD> 
<CENTER>
<H3>Summary of S2 SRM v2.2 cross test - $tim</H3>
</CENTER>
In these tests the srmCopy function is exercised. This function should
be implemented by all available Storage System by the end of the 3Q of 2007.
dCache is required to implement this function as of now.
Therefore, it is OK to have red columns for all SRM endpoints except for dCache.
However, it is not OK to have red rows since this means that a file cannot
be copied between SRMs with simple get and put operations.<BR>
<TABLE BORDER=2 CELLPADDING=1 CELLSPACING=1> 
<!-- header --> 
<TR BGCOLOR="#CCCCCC"> 
  <TH VALIGN=center ALIGN=center><FONT SIZE=3><B>SRM function</B></FONT></TH> 
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

echo "<TR><TH BGCOLOR=OLIVE COLSPAN=10 HEIGHT=40>" >> ${Index_File}
echo "Copy Tests in PUSH mode</TH></TR>" >> ${Index_File}


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
     if test $s2_num -ge 10 -a $delim -eq 1; then
       echo "<TH BGCOLOR=OLIVE COLSPAN=10 HEIGHT=40>" >> ${Index_File} 
       echo "Copy Tests in PULL mode</TH></TR><TR>" >> ${Index_File}
       delim=0
     fi
   fi
   echo "<TD VALIGN=top ALIGN=center><B>${s2}</B></TD>" >> ${Index_File}
   echo "<TD VALIGN=top ALIGN=center>   </TD>" >> ${Index_File}
   for dir in `ls -1 $S2_LOGS | grep -v index.html`;
   do
    if test -e ${S2_LOGS}/${dir}/exit.log; then
      err=`grep \(${s2_t}\.sh\) ${S2_LOGS}/${dir}/exit.log | cut -d" " -f1`
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
    if test -e ${S2_LOGS}/${dir}/${s2_t}.out; then
      echo "<A HREF=${dir}/${s2_t}.out>Out</A>" >> ${Index_File}
    fi
    if test -e ${S2_LOGS}/${dir}/${s2_t}.log; then
      echo " <A HREF=${dir}/${s2_t}.log>Log</A>" >> ${Index_File}
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

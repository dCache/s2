#!/bin/bash
#
# Useful variables
#
S2_LOGS=./s2_logs
WEB_CONF=../../web_config.conf
TEST_CONF=./description.web
tim=`date +"%A %e %B %Y %I:%M%P %Z"`
if test $# -ge 2; then
   Index_File=$1;
   Vo=$2
   vO=-${Vo}
else
   Vo=""
   vO=""
   if test $1; then
      Index_File=$1
   else
      Index_File=${S2_LOGS}/index.html
   fi
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
  <TITLE>Summary of S2 SRM v2.2 ${Vo} VO/CCRC08 tests</TITLE> 
</HEAD> 
<CENTER>
<H3>Summary of S2 SRM v2.2 ${Vo} VO/CCRC08 test - $tim</H3>
</CENTER>
This family of tests assumes a correct publication of the SRM v2.2 endpoint
in the production information system. If the endpoint is not correctly
published, the tests fail.
<TABLE BORDER=2 CELLPADDING=1 CELLSPACING=1> 
<!-- header --> 
<TR BGCOLOR="#CCCCCC"> 
  <TH VALIGN=center ALIGN=center><FONT SIZE=3><B>SRM test</B></FONT></TH> 
  <TH VALIGN=top ALIGN=center>   </TH> 
EOF
#
for dir in `ls -1 $S2_LOGS | grep -v index`;
do
  sitetag=`awk '/^'${dir}' / {print $2}' ${WEB_CONF}`
  if test "x${sitetag}" == "x"; then
     echo "Unrecognized endpoint - Exiting"; exit 1
  fi
  if test -e ${S2_LOGS}/${dir}/scheduled-downtime; then
     echo "<TH VALIGN=top ALIGN=center><FONT SIZE=3>$sitetag<BR>Scheduled downtime</FONT></TH>" >> ${Index_File}
  else
     echo "<TH VALIGN=top ALIGN=center><FONT SIZE=3><A HREF=${dir}/index${vO}.html>$sitetag</A></FONT></TH>" >> ${Index_File}
  fi
done
#
tokenlist=`ls -C1 $S2_LOGS/*/01_Ping${vO}*.e1 | awk -F- '{printf "%s %s\n",$2,$3}' | sort -u | awk '{printf "%s-%s ",$1,$2}END{printf "\n"}'`
#
echo "</TR>" >> ${Index_File}
#
echo "<TR>" >> ${Index_File}
s21="01_Ping.s2"
s2_t="01_Ping"
s2="Ping" 
#
testtag=`awk -F% '/^'${s2}'%/ {print $2}' ${TEST_CONF}`
echo "<TD VALIGN=top ALIGN=left><A HREF=../${s21}>${testtag}</A></TD>" >> ${Index_File}
echo "<TD VALIGN=top ALIGN=left>   </TD>" >> ${Index_File}
for dir in `ls -1 $S2_LOGS | grep -v index`;
do
   exitfil=`ls -C1 ${S2_LOGS}/${dir}/exit_${Vo}*.log 2>/dev/null | awk '{if (NR==1) print $0}'`
   tokenpath=`basename ${exitfil} .log | awk -F_ '{print $2}'`
   if test ${exitfil} -a -e ${exitfil}; then
     err=`grep \(${s2_t}\.sh\) ${exitfil} | cut -d" " -f1`
     if test "$err" ; then
        if test $err -eq 0 ; then
           color="#E0E0E0"
        else
           color="RED"
        fi
     else
        color="RED"
     fi
   else
     color="RED"
   fi
   if test ! -e ${S2_LOGS}/${dir}/${s2_t}-${tokenpath}.out -a ! -e ${S2_LOGS}/${dir}/${s2_t}-${tokenpath}.log; then
      color="#E0E0E0"
      echo "<TD BGCOLOR=${color} VALIGN=center ALIGN=center>" >> ${Index_File}
      echo " " >> ${Index_File}
   else
      echo "<TD BGCOLOR=${color} VALIGN=center ALIGN=center>" >> ${Index_File}
      if test ${color} = "RED"; then
         echo "<B><I>NO</B></I>("  >> ${Index_File}
      else
         echo "<B><I>YES</B></I>("  >> ${Index_File}
      fi
      if test -e ${S2_LOGS}/${dir}/${s2_t}-${tokenpath}.out; then
        echo "<A HREF=${dir}/${s2_t}-${tokenpath}.out>Out</A>" >> ${Index_File}
      fi
      if test -e ${S2_LOGS}/${dir}/${s2_t}-${tokenpath}.log; then
        echo " <A HREF=${dir}/${s2_t}-${tokenpath}.log>Log</A>" >> ${Index_File}
      fi
      echo ")"  >> ${Index_File}
   fi
      echo "</TD>" >> ${Index_File}
done
echo "</TR>" >> ${Index_File}
#
for tok in `echo $tokenlist`; 
do
#
   for s21 in `ls -1 *.s2`;
   do
      s2_t=`basename ${s21} .s2`
      s2=`echo $s2_t |cut -d_ -f2`
      if test -z "$s2" ; then
        s2="$s2_t"
      else
        s2_num=`echo $s2_t |cut -d_ -f1`
      fi
      if test "x${s2}" != "xPing"; then
         vo=`echo ${tok} | awk -F- '{print $1}' | sed 'y/abcdefghijklmnopqrstuvwyxz/ABCDEFGHIJKLMNOPQRSTUVWYXZ/'`
         toktag=`echo ${tok} | awk -F- '{print $2}'`
         tttag=`awk -F% '/^'${s2}'%/ {print $2}' ${TEST_CONF}`
         testtag="<B>$vo</B> $tttag <B>$toktag</B>" 
         echo "<TD VALIGN=top ALIGN=left><A HREF=../${s21}>${testtag}</A></TD>" >> ${Index_File}
         echo "<TD VALIGN=top ALIGN=left>   </TD>" >> ${Index_File}
         for dir in `ls -1 $S2_LOGS | grep -v index.html`;
         do
          exitfil=`ls -C1 ${S2_LOGS}/${dir}/exit_${tok}*.log 2>/dev/null | awk '{if (NR==1) print $0}'`
          tokenpath=`basename ${exitfil} .log | awk -F_ '{print $2}'`
          if test ${exitfil} -a -e ${exitfil}; then
            err=`grep \(${s2_t}\.sh\) ${exitfil} | cut -d" " -f1`
            if test "$err" ; then
               if test $err -eq 0 ; then
                  color="#E0E0E0"
               else
                  color="RED"
               fi
            else
               color="RED"
            fi
          else
            color="#E0E0E0"
          fi
          outfil=`ls -C1 ${S2_LOGS}/${dir}/${s2_t}-${tok}*.out 2>/dev/null | awk '{if (NR==1) print $0}'`
          if test ${outfil}; then
             tokenpath=`basename ${outfil} .out | sed -e 's/'${s2_t}'-//g'`
          if test ! -e ${outfil} -a ! -e ${S2_LOGS}/${dir}/${s2_t}-${tokenpath}.log; then
             color="#E0E0E0"
             echo "<TD BGCOLOR=${color} VALIGN=center ALIGN=center>" >> ${Index_File}
             echo "" >> ${Index_File}
          else
             echo "<TD BGCOLOR=${color} VALIGN=center ALIGN=center>" >> ${Index_File}
             if test ${color} = "RED"; then
                echo "<B><I>NO</B></I>("  >> ${Index_File}
             else
                echo "<B><I>YES</B></I>("  >> ${Index_File}
             fi
             if test ${outfil} -a -e ${outfil}; then
               echo "<A HREF=${dir}/${s2_t}-${tokenpath}.out>Out</A>" >> ${Index_File}
             fi
             if test -e ${S2_LOGS}/${dir}/${s2_t}-${tokenpath}.log; then
               echo " <A HREF=${dir}/${s2_t}-${tokenpath}.log>Log</A>" >> ${Index_File}
             fi
             echo ")" >> ${Index_File}
          fi
          else
             color="#E0E0E0"
             echo "<TD BGCOLOR=${color} VALIGN=center ALIGN=center>" >> ${Index_File}
             echo "" >> ${Index_File}
          fi
          echo "</TD>" >> ${Index_File}
         done
         echo "</TR>" >> ${Index_File}
      fi
   done
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

# Variables
S2_LOGS			= *.out *.log index.html
S2_LOGS_DIR		= .
S2_EXIT_LOG		:= exit.log
S2_HTML_LOG		:= index.html


# Functions
CWD			= $(shell pwd)
CLIMB			= $(shell (test -r $1 && $3 $1) || (test -r ../$1 && $3 ../$1) || (test -r ../../$1 && $3 ../../$1) || (test -r ../../../$1 && $3 ../../../$1) || (test -r ../../../../$1 && $3 ../../../../$1) || (test -r ../../../../../$1 && $3 ../../../../../$1) || (test -r ../../../../../../$1 && $3 ../../../../../../$1) || echo $2)
WHICH			= $(call CLIMB,$1,$2,echo)
NS			= $(call CLIMB,$1,$2,cat)


# HTML generation functions
#HTML_IF_FILE_HREF	 = $(if $(findstring yes,$(shell test -f $1 && echo yes)),<A HREF=$1>$2</A>,n/a)
HTML_IF_FILE_HREF	 = $(if $(findstring yes,$(shell test -f $1 && echo yes)),<A HREF=$1>$2</A>,n/a)

HTML_HEAD		= \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\
\n<HTML>\
\n<HEAD>\
\n  <TITLE>$1</TITLE>\
\n</HEAD>\
\n<H1>$1</H1>

HTML_TAIL		= \
\n</BODY>\
\n</HTML>

HTML_TABLE_HEAD		= \
\n<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=1>\
\n<!-- header -->\
\n<TR>\
\n  <TD VALIGN=top ALIGN=center><B>Script</B></TD>\
\n  <TD VALIGN=top ALIGN=center><B>Exit</B></TD>\
\n  <TD VALIGN=top ALIGN=center><B>Stdout</B></TD>\
\n</TR>\
\n<!-- results -->

HTML_TABLE_TAIL		= \
</TABLE>

s_file_log		= \
	echo -n "  <TD VALIGN=top ALIGN=center>" >> $1;\
	echo -n "|$2|$3|";\
	if test -s "$2" ; then\
	  echo -en "$3" >> $1;\
	else\
	  echo -en "$4" >> $1;\
	fi;\
	echo "</TD>" >> $1

HTML_TABLE_ROW		= \
echo -e "<TR>" >> $1;\
echo -n "  <TD VALIGN=top ALIGN=center>" >> $1; if test -s "$$s2" ; then echo -en "<A HREF=../../$$s2>$$s2_bare</A>" >> $1; else echo -en "n/a" >> $1; fi; echo "</TD>" >> $1;\
echo -n "  <TD VALIGN=top ALIGN=center>" >> $1; if test -s "$(S2_LOGS_DIR)/exit.log" ; then err=`grep "$$s2" $(S2_LOGS_DIR)/exit.log | cut -d" " -f1`; echo -en "$$err" >> $1; else echo -en "n/a" >> $1; fi; echo "</TD>" >> $1;\
echo -n "  <TD VALIGN=top ALIGN=center>" >> $1; if test -s "$(S2_LOGS_DIR)/$$s2_bare.log" ; then echo -en "<A HREF=$$s2_bare.log>stdout<A>" >> $1; else echo -en "n/a" >> $1; fi; echo "</TD>" >> $1;\
echo -e "</TR>" >> $1


# Directories
SUBDIRS			= `find . -maxdepth 1 -type d | sed '1,0 d' | sed 's|^./||' | egrep -v '^(.svn|CVS|template|include)$$'`
SUBDIRS_CLEAN		= $(SUBDIRS)


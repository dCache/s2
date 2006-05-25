# Variables
S2_EXT			:= s2
SH_EXT			:= sh
OUT_EXT			:= out
S2_LOGS			= *.p *.d *.e *.l *.w *.out *.err *.log *.e0 *.e1 *.e2 $(S2_EXIT_LOG) $(S2_HTML_LOG)
S2_FILES		:= $(wildcard *.s2)
S2_EXIT_LOG		:= exit.log
S2_HTML_LOG		:= index.html


# Functions
CWD			= $(shell pwd)
CLIMB			= $(shell (test -r $1 && $3 $1) || (test -r ../$1 && $3 ../$1) || (test -r ../../$1 && $3 ../../$1) || (test -r ../../../$1 && $3 ../../../$1) || (test -r ../../../../$1 && $3 ../../../../$1) || echo $2)
WHICH			= $(call CLIMB,$1,$2,echo)
NS			= $(call CLIMB,$1,$2,cat)


# HTML generation functions
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
\n  <TD VALIGN=top ALIGN=center><B>Before exec</B></TD>\
\n  <TD VALIGN=top ALIGN=center><B>After exec</B></TD>\
\n  <TD VALIGN=top ALIGN=center><B>stdout</B></TD>\
\n  <TD VALIGN=top ALIGN=center><B>stderr</B></TD>\
\n  <TD VALIGN=top ALIGN=center><B>Log</B></TD>\
\n  <TD VALIGN=top ALIGN=center><B>Debug</B></TD>\
\n  <TD VALIGN=top ALIGN=center><B>Custom log</B></TD>\
\n</TR>\
\n<!-- results -->

HTML_TABLE_TAIL		= \
</TABLE>

s_file_log		= \
	echo -n "  <TD VALIGN=top ALIGN=center>" >> $1;\
	if test -s "$2" ; then\
	  echo -en "$3" >> $1;\
	else\
	  echo -en "$4" >> $1;\
	fi;\
	echo "</TD>" >> $1

HTML_TABLE_ROW		= \
echo -e "<TR>" >> $1;\
$(call s_file_log,$1,$2.s2,<A HREF=$2.s2>$2</A>,n/a);\
$(call s_file_log,$1,$2.e2,<A HREF=$2.e2>$3</A>,n/a);\
$(call s_file_log,$1,$2.e0,<A HREF=$2.e0>before exec</A>,n/a);\
$(call s_file_log,$1,$2.e1,<A HREF=$2.e1>after exec</A>,n/a);\
$(call s_file_log,$1,$2.out,<A HREF=$2.out>stdout</A>,n/a);\
$(call s_file_log,$1,$2.err,<A HREF=$2.err>stderr</A>,n/a);\
$(call s_file_log,$1,$2.l,<A HREF=$2.l>log</A>,n/a);\
$(call s_file_log,$1,$2.d,<A HREF=$2.l>debug</A>,n/a);\
$(call s_file_log,$1,$2.log,<A HREF=$2.l>custom log</A>,n/a);\
echo -e "</TR>" >> $1


# Directories
SUBDIRS			= `find . -maxdepth 1 -type d | sed '1,0 d' | sed 's|^./||' | egrep -v '^(.svn|CVS|template)$$'`
SUBDIRS_CLEAN		= $(SUBDIRS)
CHECK_DIR		= check


# Scripts
S2_SH			= $(call WHICH,bin/s2.sh,false)

-include local.mk ../local.mk ../../local.mk ../../../mk/local.mk ../../../../mk/local.mk

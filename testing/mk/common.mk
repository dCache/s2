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
HTML_HEAD		= "\
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n\
<HTML>\n\
<HEAD>\n\
  <TITLE>$1</TITLE>\n\
</HEAD>\n\
<H1>$1</H1>\n"

HTML_TAIL		= "\n\
</BODY>\n\
</HTML>"

HTML_TABLE_HEAD		= "\
<TABLE BORDER=1 CELLPADDING=1 CELLSPACING=1>\n\
<!-- header -->\n\
<TR>\n\
  <TD VALIGN=top ALIGN=center><B>Script</B></TD>\n\
  <TD VALIGN=top ALIGN=center><B>Exit</B></TD>\n\
  <TD VALIGN=top ALIGN=center><B>Before exec</B></TD>\n\
  <TD VALIGN=top ALIGN=center><B>After exec</B></TD>\n\
  <TD VALIGN=top ALIGN=center><B>After eval</B></TD>\n\
  <TD VALIGN=top ALIGN=center><B>Log</B></TD>\n\
  <TD VALIGN=top ALIGN=center><B>Debug</B></TD>\n\
</TR>\n\
<!-- results -->"

HTML_TABLE_TAIL		= "\
</TABLE>"

HTML_TABLE_ROW		= "\
<TR>\n\
  <TD VALIGN=top><A HREF=$1.s2>$1.s2</A></TD>\n\
  <TD VALIGN=top>$2</TD>\n\
  <TD VALIGN=top><A HREF=$1.e0>$1.e0</A><BR></TD>\n\
  <TD VALIGN=top><A HREF=$1.e1>$1.e1</A><BR></TD>\n\
  <TD VALIGN=top><A HREF=$1.e2>$1.e2</A><BR></TD>\n\
  <TD VALIGN=top><A HREF=$1.l>$1.l</A><BR></TD>\n\
  <TD VALIGN=top><A HREF=$1.d>$1.d</A><BR></TD>\n\
</TR>"


# Directories
SUBDIRS			= `find . -maxdepth 1 -type d | sed '1,0 d' | sed 's|^./||' | egrep -v '^(.svn|CVS|template)$$'`
SUBDIRS_CLEAN		= $(SUBDIRS)
CHECK_DIR		= check


# Scripts
S2_SH			= $(call WHICH,bin/s2.sh,false)

-include local.mk ../local.mk ../../local.mk ../../../mk/local.mk ../../../../mk/local.mk

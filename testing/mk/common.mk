# Variables
S2_EXT			:= s2
SH_EXT			:= sh
S2_LOGS			:= *.p *.d *.e *.l *.w *.out *.err *.log *.e0 *.e1 *.e2
S2_FILES		:= $(wildcard *.s2)
#S2_TEST_REPORT		:= s2_report-$(shell date '+%Y-%m-%d@%H:%M').log
S2_TEST_REPORT		:= s2_report.log


# Functions
CWD		= $(shell pwd)
ALBUM_DIR	= $(notdir $(CWD))
CLIMB		= $(shell (test -r $1 && $3 $1) || (test -r ../$1 && $3 ../$1) || (test -r ../../$1 && $3 ../../$1) || (test -r ../../../$1 && $3 ../../../$1) || (test -r ../../../../$1 && $3 ../../../../$1) || echo $2)
WHICH		= $(call CLIMB,$1,$2,echo)
NS		= $(call CLIMB,$1,$2,cat)


# Directories
SUBDIRS		= `find . -maxdepth 1 -type d | sed '1,0 d' | sed 's|^./||' | egrep -v '^(.svn|CVS|template)$$'`
SUBDIRS_CLEAN	= `find . -maxdepth 1 -type d | sed '1,0 d' | sed 's|^./||' | egrep -v '^(.svn|CVS|template)$$'`


# Scripts
S2_SH		= $(call WHICH,bin/s2.sh,false)

-include local.mk ../local.mk ../../local.mk ../../../mk/local.mk ../../../../mk/local.mk

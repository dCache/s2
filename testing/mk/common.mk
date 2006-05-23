# Variables
S2_EXT			:= s2
SH_EXT			:= sh
OUT_EXT			:= out
S2_LOGS			:= *.p *.d *.e *.l *.w *.out *.err *.log *.e0 *.e1 *.e2
S2_FILES		:= $(wildcard *.s2)
S2_EXIT_LOG		:= exit.log


# Functions
CWD		= $(shell pwd)
CLIMB		= $(shell (test -r $1 && $3 $1) || (test -r ../$1 && $3 ../$1) || (test -r ../../$1 && $3 ../../$1) || (test -r ../../../$1 && $3 ../../../$1) || (test -r ../../../../$1 && $3 ../../../../$1) || echo $2)
WHICH		= $(call CLIMB,$1,$2,echo)
NS		= $(call CLIMB,$1,$2,cat)


# Directories
SUBDIRS		= `find . -maxdepth 1 -type d | sed '1,0 d' | sed 's|^./||' | egrep -v '^(.svn|CVS|template)$$'`
SUBDIRS_CLEAN	= $(SUBDIRS)
CHECK_DIR	= check


# Scripts
S2_SH		= $(call WHICH,bin/s2.sh,false)

-include local.mk ../local.mk ../../local.mk ../../../mk/local.mk ../../../../mk/local.mk

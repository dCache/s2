-include mk/general.mk ../mk/general.mk ../../mk/general.mk ../../../mk/general.mk ../../../../mk/general.mk ../../../../../mk/general.mk ../../../../../../mk/general.mk

SUBDIRS		= `find . -maxdepth 1 -type d | sed '1,0 d' | sed 's|^./||' | egrep -v '^(.svn|CVS|template|include)$$'`
SUBDIRS_CLEAN	= $(SUBDIRS)

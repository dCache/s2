-include ./common.mk

ifeq ($(shell test -n "$(S2_FILES)" && echo x),)
-include mk/subdirs.mk ../mk/subdirs.mk ../../mk/subdirs.mk ../../../mk/subdirs.mk ../../../../mk/subdirs.mk ../../../../../mk/subdirs.mk ../../../../../../mk/subdirs.mk
else
-include ./s2.mk
endif

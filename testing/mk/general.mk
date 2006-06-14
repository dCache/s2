-include mk/common.mk ../mk/common.mk ../../mk/common.mk ../../../mk/common.mk ../../../../mk/common.mk ../../../../../mk/common.mk ../../../../../../mk/common.mk

ifeq ($(shell test -n "$(S2_FILES)" && echo x),)
-include mk/subdirs.mk ../mk/subdirs.mk ../../mk/subdirs.mk ../../../mk/subdirs.mk ../../../../mk/subdirs.mk ../../../../../mk/subdirs.mk ../../../../../../mk/subdirs.mk
else
-include mk/s2.mk ../mk/s2.mk ../../mk/s2.mk ../../../mk/s2.mk ../../../../mk/s2.mk ../../../../../mk/s2.mk ../../../../../../mk/s2.mk
endif

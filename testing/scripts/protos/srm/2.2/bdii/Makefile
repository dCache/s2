-include common.mk

.PHONY:	test

all: test

help:
	@echo "make [option]"
	@echo "~~~~~~~~~~~~~"
	@echo "help                  this help"
	@echo "test                  run all tests"
	@echo "clean                 cleanup"

test: clean
	@if test ! -d $(S2_LOGS_DIR) ; then \
	   mkdir -p $(S2_LOGS_DIR) 2>/dev/null; \
	fi;
	@echo -e "$(call HTML_HEAD,`basename $(CWD)`)" >> $(S2_LOGS_DIR)/index.html;
	@echo -e "$(call HTML_TABLE_HEAD)" >> $(S2_LOGS_DIR)/index.html;
	@hst=`sed -n '/^\s*'$(S2_TEST_SITE)')/ s/.*SRM_ENDPOINT=srm:\/\/\(.*\):.*/\1/gp' ../../s2.env`; \
	for s2 in `ls -1 [A-Z]*.sh 2>/dev/null` ; do \
	  s2_bare=`basename $$s2 .sh` ; \
	  if test $$s2_bare != "make_result_web_page" ; then \
	      s2_out=$(S2_LOGS_DIR)/$$s2_bare.log ; \
              echo -n $$s2; \
	      time ./$$s2 $$hst >> $$s2_out; \
	      err=$$?; \
	      echo "$$err"; echo ""; \
	      echo "$$err ($$s2)" >> $(S2_LOGS_DIR)/exit.log; \
	      $(call HTML_TABLE_ROW,$(S2_LOGS_DIR)/index.html); \
	  fi; \
	done;
	@echo -e "$(call HTML_TABLE_TAIL)" >> $(S2_LOGS_DIR)/index.html;
	@echo -e "$(call HTML_TAIL)" >> $(S2_LOGS_DIR)/index.html

clean:
	@rm -f $(S2_LOGS)
	@cd $(S2_LOGS_DIR) 2>/dev/null; rm -f $(S2_LOGS)


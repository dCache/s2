######################################################################
# Build process directories
LOG_DIR		= log
RULES_FILE	= rules
PACKAGES_FILE	= packages
PRJ_DIR		= projects
BUILD_DIR	= builds

### Rules ############################################################
.PHONY: Makefile

all:
	@echo "Please select one of the following targets"
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@find $(PRJ_DIR) -name $(RULES_FILE) -exec dirname {} \; |  sed "s|^$(PRJ_DIR)/||" | sort


### Projects #########################################################

%: FORCE
	pant.pl -i $(subst /,_,$@).html \
	        -b $(BUILD_DIR)/$@ \
	        -l $(LOG_DIR) \
	        $(PRJ_DIR)/$@

FORCE: ;

### Cleanup ##########################################################
.PHONY: clean distclean mclean

clean distclean mclean: clean
	-rm -rf $(LOG_DIR) *.html
	-rm -rf $(BUILD_DIR)

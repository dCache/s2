SHELL		:= /bin/sh

# Include local configuration file (generated by ./configure) ########
_CONFIG_MAK	:= ./config.mak
sinclude $(_CONFIG_MAK)


# Makefile variables #################################################
TODAY		:= $(shell date '+%Y-%m-%d')
VERSION_H	:= include/version.h


# Subdirectories #####################################################
SUBDIRS		:= libcommon libmatch
ifneq ($(SRM_VERSION),)
SRM_DIR		:= protos/srm/$(SRM_VERSION)
SUBDIRS		+= $(SRM_DIR)/gsoap $(SRM_DIR)/api $(SRM_DIR)/n
endif
SUBDIRS		+= libexpr libptree src doc testing
SUBDIRS_CLEAN	:= $(SUBDIRS) pant www


# Documentation ######################################################
DOCS		:= \
  AUTHORS\
  BUGS\
  ChangeLog\
  COPYING\
  FAQ\
  INSTALL\
  README.md\
  README.s2\
  TODO\


# Test cases #########################################################
TEST_DIR		:= testing


# Rules ##############################################################
.PHONY: Makefile

ifneq ($(CONFIG_MAK),)
all: $(VERSION_H) $(SUBDIRS)
	@for dir in $(SUBDIRS) ;\
	do \
	  make -C "$$dir" || exit $$? ;\
	done
endif

$(_CONFIG_MAK):
	@echo "Please run ./configure to generate $(_CONFIG_MAK)" >&2
	@exit 1


# Install ############################################################
install: install-subdirs install-docs install-tests

install-subdirs: $(SUBDIRS)
	@for dir in $(SUBDIRS) ;\
	do \
	  make -C "$$dir" install || exit $$? ;\
	done

install-docs: $(DOCS)
	@if test -n "$(DOCS)" ; then\
	  mkdir -p $(prefix)$(_docdir) ;\
	  install -m 0644 $(DOCS) $(prefix)$(_docdir) ;\
	fi

install-tests: $(TEST_DIR)
	@if test -n "$(TEST_DIR)" ; then\
	  mkdir -p $(prefix)$(_docdir) ;\
	  cp -R $(TEST_DIR) $(prefix)$(_docdir) ;\
	fi


# RPM ################################################################
RPM_SPECIN	:= rpm.spec.in
RELEASE		:= $(RPM_PACKAGE)-$(VERSION)
RPMTOPDIR	:= $(shell rpm --eval '%_topdir')
BUILDDIR	:= $(shell rpm --eval '%_builddir')
BUILDROOT	:= $(shell rpm --eval '%_tmppath')/$(RELEASE)-buildroot
RPMBUILD	:= rpmbuild 

rpm: rpm_build rpm_show

rpm_dirs:
	mkdir -p $(RPMTOPDIR)/{,BUILD,RPMS,SOURCES,SPECS,SRPMS} \
	         $(RPMTOPDIR)/RPMS/{i386,i586,i686,noarch} \
		 $(RPMTOPDIR)/tmp

dot_configure:
	touch .configure

rpm_include: dot_configure $(RPM_SPECIN)
	sed\
	  -e '/^%include .configure/r .configure'\
	  -e '/^%include .rpmdef/r .rpmdef'\
	  -e '/^%include .rpmfiles/r .rpmfiles'\
	  -e '/^%include .rpmreq/r .rpmreq'\
	  < $(RPM_SPECIN) | grep -v '^%include' > $(PACKAGE).spec

tar: rpm_include rpmclean
	-rm -rf $(BUILDROOT)
	-rm -rf $(BUILDDIR)/$(RELEASE)
	mkdir $(BUILDDIR)/$(RELEASE)
	cp -r * $(BUILDDIR)/$(RELEASE)
	cd $(BUILDDIR) ; tar zcvf $(RELEASE).tar.gz\
	  --exclude=.depend --exclude='$(RELEASE)/*/CVS' --exclude='$(RELEASE)/*/.cvsignore'\
	  --exclude='*~' --exclude='#*#'\
	  --exclude=1gnore --exclude=./pant/ --exclude=www\
	  $(RELEASE)
	mv $(BUILDDIR)/$(RELEASE).tar.gz $(RPMTOPDIR)/SOURCES

rpm_build: rpm_dirs tar rpm_include
	$(RPMBUILD) -ta $(RPMTOPDIR)/SOURCES/$(RELEASE).tar.gz

rpm_show: 
	@echo "You have now:" ; ls -l $(RPMTOPDIR)/*RPMS/*/*.rpm


# Versioning #########################################################
$(VERSION_H):
	@echo $(VERSION) > VERSION
	@echo "#ifndef MK_VERSION" > $(VERSION_H)
	@echo "#define MK_VERSION \"$(VERSION)\"" >> $(VERSION_H)
	@echo "#endif /* MK_VERSION */" >> $(VERSION_H)
	@echo >> $(VERSION_H)
	@echo "#ifndef BUILD_DATE" >> $(VERSION_H)
	@echo "#define BUILD_DATE \"$(TODAY)\"" >> $(VERSION_H)
	@echo "#endif /* BUILD_DATE */" >> $(VERSION_H)


# CVS/Subversion #####################################################
cvsignore:
	@for f in `find . -name .cvsignore` ;\
	do \
	  svn propset svn:ignore -F "$$f" `dirname "$$f"` ;\
	done

ifeq ($(svnroot),)
tag: $(tagfile) $(_CONFIG_MAK)
	@echo "commit..."
	cvs commit
	@echo "tag..."
	cvs rtag v$(subst .,_,$(shell cat $(tagfile))) $(CVS_REPOSITORY)
else
tag: $(tagfile) $(_CONFIG_MAK)
	@echo "commit..."
	svn commit
	@echo "tag..."
	svn copy $(svnroot)/trunk \
	         $(svnroot)/tags/$(shell cat $(tagfile))
endif


# Cleanup ############################################################
.PHONY: clean clean-subdirs dependclean cvsclean svnclean rpmclean distclean mclean

clean: clean-subdirs
	-find locale \( -name *.po[~t] -or -name *.mo \) | xargs rm -f
	-rm -f *.log

clean-subdirs:
	@for dir in $(SUBDIRS_CLEAN) ;\
	do \
	  if test -d "$$dir" ; then\
	    make -C "$$dir" clean || exit $$? ;\
	  fi \
	done

dependclean:
	-find . -name .depend -type f -exec rm -rf {} \;

cvsclean:
	-find . -name CVS -type d -exec rm -rf {} \;

svnclean:
	-find . -name .svn -type d -exec rm -rf {} \;

rpmclean: clean

update:
	cvs update -d
	rm -rf tests

distclean mclean: clean
	@for dir in $(SUBDIRS_CLEAN) ;\
	do \
	  if test -d "$$dir" ; then\
	    make -C "$$dir" distclean || exit $$? ;\
	  fi \
	done
	-rm -f $(CONFIG_MAK) $(CONFIG_H) $(VERSION_H) VERSION .configure .rpm* *.spec *.log

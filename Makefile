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

# Scripts to be copied directly ######################################
SCRIPTS_DIR		:= scripts

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
install: install-subdirs install-docs install-tests install-scripts

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
	  mkdir -p $(prefix)$(_datadir)/s2 ;\
	  cp -R $(TEST_DIR) $(prefix)$(_datadir)/s2 ;\
	fi

install-scripts: $(SCRIPTS_DIR)
	@if test -n "$(SCRIPTS_DIR)" ; then\
	  mkdir -p $(prefix)$(_bindir) ;\
	  install -m 0755 -t $(prefix)$(_bindir) $(SCRIPTS_DIR)/* ;\
	fi


# RPM ################################################################
RPM_SPECIN	:= rpm.spec.in
RELEASE		:= $(RPM_PACKAGE)-$(VERSION)
RPMTOPDIR	:= $(shell pwd)/RPM-BUILD
BUILDDIR	:= $(RPMTOPDIR)/BUILD
RPMTMPDIR	:= $(RPMTOPDIR)/TMP
RPMSOURCESDIR	:= $(RPMTOPDIR)/SOURCES
BUILDROOT	:= $(RPMTMPDIR)/$(RELEASE)-buildroot
RPMBUILD	:= rpmbuild

rpm: $(_CONFIG_MAK) rpm_build rpm_show

rpm_dirs:
	mkdir -p $(RPMTOPDIR) \
	         $(RPMTMPDIR) \
	         $(BUILDDIR) \
	         $(RPMSOURCESDIR) \
	         $(RPMTOPDIR)/RPMS \
	         $(RPMTOPDIR)/SPECS \
	         $(RPMTOPDIR)/SRPMS \
	         $(RPMTOPDIR)/RPMS/i386 \
	         $(RPMTOPDIR)/RPMS/i586 \
	         $(RPMTOPDIR)/RPMS/i686 \
	         $(RPMTOPDIR)/RPMS/x86_64 \
	         $(RPMTOPDIR)/RPMS/noarch

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
	cp -r -t $(BUILDDIR)/$(RELEASE) $(shell ls|grep -v RPM-BUILD)
	cd $(BUILDDIR) ; tar zcvf $(RELEASE).tar.gz\
	  --exclude=.depend --exclude='$(RELEASE)/*/CVS' --exclude='$(RELEASE)/*/.cvsignore'\
	  --exclude='*~' --exclude='#*#'\
	  --exclude=1gnore --exclude=./pant/ --exclude=www\
	  $(RELEASE)
	mv $(BUILDDIR)/$(RELEASE).tar.gz $(RPMSOURCESDIR)

rpm_build: rpm_dirs tar rpm_include
	$(RPMBUILD) $(RPMBUILD_EXTRA_OPTIONS) -D "_topdir $(RPMTOPDIR)" -D "_tmpdir $(RPMTMPDIR)" -ta $(RPMSOURCESDIR)/$(RELEASE).tar.gz

# Work-around broken rpmbuild that don't accept -D -- The above command works
# when building as user root; we move the packages back to where we expect
# them to be
	@if test ! -f $(RPMTOPDIR)/*RPMS/*/*.rpm -a -f /usr/src/redhat/RPMS/*/s2-*.rpm; then \
	  mv /usr/src/redhat/RPMS/*/s2-*.rpm $(RPMTOPDIR)/RPMS/x86_64 ;\
	  mv /usr/src/redhat/SRPMS/s2-*.src.rpm $(RPMTOPDIR)/SRPMS ;\
	fi

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
	-rm -rf $(RPMTOPDIR)
	-rm -f $(CONFIG_MAK) $(CONFIG_H) $(VERSION_H) VERSION .configure .rpm* *.spec *.log

SHELL		:= /bin/sh

# Include local configuration file (generated by ./configure) ########
_CONFIGURE_DIR	:= ..
_CONFIGURE	:= $(_CONFIGURE_DIR)/configure
_CONFIG_MAK	:= $(_CONFIGURE_DIR)/config.mak
sinclude $(_CONFIG_MAK)


# Sources ############################################################
SRCS		= $(wildcard *.cpp)


# Objects ############################################################
OBJS		+= s2.o


# Executables ########################################################
TEST		= s2
BINS		= $(TEST)$(BINSUF)


# Documentation ######################################################
DOCS		:=


# Includes ###########################################################
CPPFLAGS        += $(INCLUDES)
INCS		:= 


# Libraries ##########################################################
LIB		= lib$(PACKAGE)$(LIBSUF)
ifeq ($(_enable_shared),yes)
SLIB		= lib$(PACKAGE)$(SLIBSUF)
endif
#LIBS		= $(LIB) $(SLIB)


# Extra libraries ####################################################
ifeq ($(_static_globus),yes)
LIB_GLOBUS		= \
  -Wl,-Bstatic \
  -lglobus_gssapi_gsi_$(GLOBUS_FLAVOUR) \
  -lglobus_gss_assist_$(GLOBUS_FLAVOUR) \
  -lglobus_common_$(GLOBUS_FLAVOUR) \
  -lglobus_gsi_callback_$(GLOBUS_FLAVOUR) \
  -lglobus_gsi_cert_utils_$(GLOBUS_FLAVOUR) \
  -lglobus_gsi_credential_$(GLOBUS_FLAVOUR) \
  -lglobus_gsi_proxy_core_$(GLOBUS_FLAVOUR) \
  -lglobus_gsi_sysconfig_$(GLOBUS_FLAVOUR) \
  -lglobus_oldgaa_$(GLOBUS_FLAVOUR) \
  -lglobus_openssl_error_$(GLOBUS_FLAVOUR) \
  -lglobus_openssl_$(GLOBUS_FLAVOUR) \
  -lglobus_proxy_ssl_$(GLOBUS_FLAVOUR) \
  -lssl_$(GLOBUS_FLAVOUR) \
  -lcrypto_$(GLOBUS_FLAVOUR) \
  -Wl,-Bdynamic
endif

PRJLIBS		:= \
  -L../libcommon \
  -L../libexpr \
  -L../libmatch \
  -L../libptree \
  -lptree \
  -lexpr \
  -lmatch \
  $(LIB_PCRE)

ifneq ($(SRM_VERSION),)
SRM_DIR		:= ../protos/srm/$(SRM_VERSION)
PRJLIBS		+= \
  -L$(SRM_DIR)/api \
  -L$(SRM_DIR)/n \
  -L$(_with_gsoapdir)/lib \
  -L$(_with_cgsi_plugindir)/lib \
  -L$(_with_globusdir)/lib \
  -lsrm2n
endif

PRJLIBS		+= \
  -lcommon

ifneq ($(SRM_VERSION),)
GSOAP_LOCAL		= $(SRM_DIR)/gsoap
OBJ_GSOAP		= $(GSOAP_LOCAL)/soapC.o $(GSOAP_LOCAL)/soapClient.o
PRJLIBS		+= \
  $(OBJ_GSOAP) \
  $(LIB_GSOAP) \
  -lsrm2api \
  $(LIB_CGSI_PLUGIN) \
  $(LIB_GLOBUS)
INCLUDES := $(INCLUDES) -I../protos/srm/include
endif

EXTRALIBS	 += $(PRJLIBS)


# Extra libraries ####################################################
LDLIBS		:= $(EXTRALIBS)


# Rules ##############################################################
.PHONY: Makefile

ifneq ($(CONFIG_MAK),)
all: depend $(LIBS) $(BINS)
endif

$(_CONFIG_MAK):
	@echo "Please run ./configure to generate $(_CONFIG_MAK)" >&2
	@exit 1

$(LIB): $(OBJS)
	rm -f $@
	$(AR) rc $@ $^
	$(RANLIB) $@

$(SLIB): $(OBJS)
	$(CC) $(SHFLAGS) \
	  -o $@ $(OBJS)


# Generate dependencies ##############################################
dep depend: .depend

.depend: $(SRCS)
	$(CC) -MM $(CPPFLAGS) $^ 1>.depend

ifneq ($(wildcard .depend),)
include .depend
endif


# Install ############################################################
install: install-bins install-docs install-libs install-incs

install-bins: $(BINS)
	@if test -n "$(BINS)" ; then \
	  mkdir -p $(prefix)$(_bindir) ; \
	  install -m 0755 $(BINS) $(prefix)$(_bindir) ; \
	fi

install-docs: $(DOCS)
	@if test -n "$(DOCS)" ; then \
	  mkdir -p $(prefix)$(_docdir) ; \
	  install -m 0644 $(DOCS) $(prefix)$(_docdir) ; \
	fi

install-incs: $(INCS)
	@for dir in $(INCS) ; \
	do \
	  mkdir -p $(prefix)$(_includedir)/`dirname $$dir` ; \
	  install -m 0644 $(INCS) $(prefix)$(_includedir)/`dirname $$dir` ; \
	done

install-libs: $(LIBS)
	@if test -n "$(LIBS)" ; then \
	  mkdir -p $(prefix)$(_libdir) ; \
	  install -m 0644 $(LIBS) $(prefix)$(_libdir) ; \
	fi


# Cleanup ############################################################
.PHONY: clean distclean mclean

clean: 
	-rm -f *.o .libs $(LIBS) $(BINS)

distclean mclean: clean
	-rm -f `basename $(CONFIG_H)` $(CONFIG_MAK) $(VERSION_H) .depend 

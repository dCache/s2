#!/bin/bash
ProgramName=$(basename $0)

######################################################################
# CONFIG_MAK, CONFIG_H and RPMBUILD functions                        #
######################################################################
l_CONFIG_MAK() {
  echo "
# less standard libraries
GLOBUS_FLAVOUR	:= gcc32dbgpthr
LIB_GLOBUS	:= ${LIB_GLOBUS}
LIB_GSOAP	:= ${LIB_GSOAP}
LIB_CGSI_PLUGIN	:= ${LIB_CGSI_PLUGIN}" >> ${CONFIG_MAK}
}

l_CONFIG_H() {
#  h_def_nl "$enable_globus_sdk" HAVE_GLOBUS "Globus SDK library"
  h_def_nl "$enable_gsoap" HAVE_GSOAP "gSOAP library"
  h_def_nl "$enable_cgsi_plugin" HAVE_CGSI_PLUGIN "CGSI plugin library"
  h_def_nl "$enable_gfal" HAVE_GFAL "GFAL library"
}

l_RPMBUILD() {
  l_RPMREQ
  l_RPMFILES
}

l_RPMREQ() {
  cat > .rpmreq << EOF
Requires:	glibc  libstdc++  libgcc
EOF

  if test x${enable_gsoap} = xyes && test x${static_globus} != xyes ; then
  cat >> .rpmreq << EOF
Requires:	vdt_globus_essentials
EOF
  fi

  if test x${enable_gsoap} = xyes && test x${static_cgsi_plugin} != xyes ; then
  cat >> .rpmreq << EOF
Requires:	CGSI_gSOAP_2.7 >= 1.1.9
EOF
  fi

  if test x${static_pcre} != xyes ; then
  cat >> .rpmreq << EOF
Requires:	pcre >= 4.4
EOF
  fi

  if test x${static_dg} != xyes ; then
  cat >> .rpmreq << EOF
Requires:	diagnose >= 0.3.8
%endif
EOF
  fi

#BuildRequires:	vdt_globus_sdk
  cat >> .rpmreq << EOF
BuildRequires:	vdt_globus_essentials
BuildRequires:	gsoap = 2.7.2
BuildRequires:	CGSI_gSOAP_2.7-dev >= 1.1.9
BuildRequires:	pcre-devel >= 4.4
BuildRequires:	libdiagnose >= 0.3.8
EOF

#Requires:	CGSI_gSOAP_2.7  vdt_globus_essentials  globus-initialization  gpt
#BuildRequires:	diagnose >= 0.3.8  pcre-devel >= 4.4
#BuildRequires:	CGSI_gSOAP-devel >= 2.7  vdt_globus_essentials >= VDT1.2.2rh9-1  gpt >= VDT1.2.2rh9-1  globus-initialization >= 2.2.4-5  vdt_globus_sdk >= VDT1.2.2rh9-1
#glibc-2.3.2-95.33
#libgcc-3.2.3-49
#libstdc++-3.2.3-49
#vdt_globus_essentials-VDT1.2.2rh9-1
}

l_RPMFILES() {
  cat > .rpmfiles << EOF
%defattr(-,root,root)

# test
%{_bindir}/
EOF

  if test x${enable_gsoap} = xyes ; then
  cat >> .rpmfiles << EOF

# headers
%{_includedir}/srm2api.h

# libraries
%{_libdir}/libsrm2api.a

# manual pages
%{_mandir}/
EOF
  fi

  cat >> .rpmfiles << EOF

## documents
%doc %{_docdir}/AUTHORS
%doc %{_docdir}/BUGS
%doc %{_docdir}/COPYING
%doc %{_docdir}/ChangeLog
%doc %{_docdir}/INSTALL
%doc %{_docdir}/README
%doc %{_docdir}/TODO
%doc %{_docdir}/s2.txt
EOF

  if test x${enable_gsoap} = xyes ; then
  cat >> .rpmfiles << EOF
%doc %{_docdir}/api/
EOF
  fi

  cat >> .rpmfiles << EOF
%doc %{_docdir}/testing/
EOF
}

######################################################################
# have_* functions                                                   #
######################################################################
have_globus() {
  cat > $TMPC << EOF
int main(int argc, char **argv) {
  return 0;	/* just try linking against globus libraries */
}
EOF
  cat $TMPC >> ${CONFIGURE_LOG}

  for try_globusdir in ${_with_globusdir} /opt/globus
  do
    ${HAVE_SH} \
      $CC $INCLUDES -I${try_globusdir}/include \
      -o $TMPE $TMPC -L${try_globusdir}/lib ${LIB_GLOBUS} ${EXTRALIBS} \
      >>${CONFIGURE_LOG} 2>&1
    if test $? -eq 0 ; then
      run $TMPE && have_globus="yes" && break || have_globus="no"
    else
      have_globus="no"
    fi
  done

  if test x$enable_globus = xyes ; then
    enable_globus=$have_globus
  fi
  if test x$enable_globus = xyes ; then
#    if test x$static_globus = xyes ; then
#      LIB_GLOBUS="-Wl,-Bstatic ${LIB_GLOBUS} -Wl,-Bdynamic"
#    fi
    if test x$try_globusdir != x${_with_globusdir} ; then
      _with_globusdir=$try_globusdir
    fi
#    INCLUDES="${INCLUDES} -I${_with_globusdir}/include"
#    EXTRALIBS="-I${_with_globusdir}/lib ${LIB_GLOBUS}"
  fi

  contains ${CONFIG_HAVE} globus
  if test "$?" -eq 0 && test x$enable_globus = xno; then
    die 1 "Globus library disabled; required by ${CONFIG_HAVE}"
  fi
}

have_globus_sdk() {
  cat > $TMPC << EOF
#include <${GLOBUS_FLAVOUR}/defs.h>
int main(int argc, char **argv) {
  return 0;
}
EOF
  cat $TMPC >> ${CONFIGURE_LOG}

  for try_globusdir in ${_with_globusdir} /opt/globus
  do
    ${HAVE_SH} \
      $CC $INCLUDES -I${try_globusdir}/include \
      -o $TMPE $TMPC -L${try_globusdir}/lib ${LIB_GLOBUS_SDK} ${EXTRALIBS} \
      >>${CONFIGURE_LOG} 2>&1
    if test $? -eq 0 ; then
      run $TMPE && have_globus_sdk="yes" && break || have_globus_sdk="no"
    else
      have_globus_sdk="no"
    fi
  done

  if test x$enable_globus_sdk = xyes ; then
    enable_globus_sdk=$have_globus_sdk
  fi
  if test x$enable_globus_sdk = xyes ; then
#    if test x$static_globus_sdk = xyes ; then
#      LIB_GLOBUS_SDK="-Wl,-Bstatic ${LIB_GLOBUS_SDK} -Wl,-Bdynamic"
#    fi
    if test x$try_globusdir != x${_with_globusdir} ; then
      _with_globusdir=$try_globusdir
    fi
#    INCLUDES="${INCLUDES} -I${_with_globusdir}/include"
#    EXTRALIBS="-I${_with_globusdir}/lib ${LIB_GLOBUS}"
  fi

  contains ${CONFIG_HAVE} globus_sdk
  if test "$?" -eq 0 && test x$enable_globus = xno; then
    die 1 "Globus SDK library disabled; required by ${CONFIG_HAVE}"
  fi
}

have_gsoap() {
  cat > $TMPC << EOF
#include <stdsoap2.h>
//#import "stlvector.h"		/* srm_211.h needs it */
int main(int argc, char **argv) {
  struct soap soap;
  return 0;
}
EOF
  cat $TMPC >> ${CONFIGURE_LOG}

  for try_gsoapdir in ${_with_gsoapdir} /opt/gsoap /opt/gsoap-slc3/2.7
  do
    ${HAVE_SH} \
      $CC $INCLUDES -I${try_gsoapdir}/include \
      -o $TMPE $TMPC -L${try_gsoapdir}/lib ${LIB_GSOAP} ${EXTRALIBS} \
      >>${CONFIGURE_LOG} 2>&1
    if test $? -eq 0 ; then
      run $TMPE && have_gsoap="yes" && break || have_gsoap="no"
    else
      have_gsoap="no"
    fi
  done

  if test x$enable_gsoap = xyes ; then
    enable_gsoap=$have_gsoap
  fi
  if test x$enable_gsoap = xyes ; then
    if test x$static_gsoap = xyes ; then
      LIB_GSOAP="-Wl,-Bstatic ${LIB_GSOAP} -Wl,-Bdynamic"
    fi
    if test x$try_gsoapdir != x${_with_gsoapdir} ; then
      _with_gsoapdir=$try_gsoapdir
    fi
#    INCLUDES="${INCLUDES} -I${_with_gsoapdir}/include"
#    EXTRALIBS="-I${_with_gsoapdir}/lib ${LIB_GSOAP}"
  fi

  contains ${CONFIG_HAVE} gsoap
  if test "$?" -eq 0 && test x$enable_gsoap = xno; then
    die 1 "gSOAP library 2.7.2 disabled; required by ${CONFIG_HAVE}"
  fi
}

have_cgsi_plugin() {
  cat > $TMPC << EOF
#include <cgsi_plugin.h>
int main(int argc, char **argv) {
  return 0;
}
EOF
  cat $TMPC >> ${CONFIGURE_LOG}

  for try_cgsi_plugindir in ${_with_cgsi_plugindir} ${_with_gsoapdir}
  do
    ${HAVE_SH} \
      $CC $INCLUDES -I${try_cgsi_plugindir}/include \
      -o $TMPE $TMPC -L${try_cgsi_plugindir}/lib ${LIB_CGSI_PLUGIN} ${EXTRALIBS} \
      >>${CONFIGURE_LOG} 2>&1
    if test $? -eq 0 ; then
      run $TMPE && have_cgsi_plugin="yes" && break || have_cgsi_plugin="no"
    else
      have_cgsi_plugin="no"
    fi
  done

  if test x$enable_cgsi_plugin = xyes ; then
    enable_cgsi_plugin=$have_cgsi_plugin
  fi
  if test x$enable_cgsi_plugin = xyes ; then
    if test x$static_cgsi_plugin = xyes ; then
      LIB_CGSI_PLUGIN="-Wl,-Bstatic ${LIB_CGSI_PLUGIN} -Wl,-Bdynamic"
    fi
    if test x$try_cgsi_plugindir != x${_with_gsoapdir} ; then
      _with_gsoapdir=$try_gsoapdir
#      INCLUDES="${INCLUDES} -I${try_cgsi_plugindir}/include"
#      EXTRALIBS="${EXTRALIBS} -L${try_cgsi_plugindir}/lib"
    fi
  fi

  contains ${CONFIG_HAVE} cgsi_plugin
  if test "$?" -eq 0 && test x$enable_cgsi_plugin = xno; then
    die 1 "CGSI plugin disabled; required by ${CONFIG_HAVE}"
  fi
}

######################################################################
# main()-closely-related functions                                   #
######################################################################
l_add_opts() {
  $_NOP
}

l_add_withdirs() {
  # compile-time directories
  add_withdir globus 'Globus' '${_with_globusdir:-${_prefix}/globus}'
  add_withdir gsoap 'gSOAP' '${_with_gsoapdir:-${_prefix}}'
  add_withdir cgsi_plugin 'CGSI plugin' '${_with_cgsi_plugin:-${_prefix}}'
}

l_add_statics() {
  add_static globus 'globus' 'Globus library' '${static_globus:-no}'				# "yes" or "no"
  add_static gsoap 'gsoap' 'gSOAP library' '${static_gsoap:-yes}'				# "yes" or "no"
  add_static cgsi_plugin 'cgsi-plugin' 'CGSI plugin' '${static_cgsi_plugin:-yes}'		# "yes" or "no"
  add_static gfal 'gfal' 'GFAL library' '${static_gfal:-no}'					# "yes" or "no"
}

l_add_enables() {
  add_enable srm2 'srm2' 'SRM2 support' '${enable_srm2:-yes}'					# "yes" or "no"
  add_enable globus 'globus' 'Globus essentials library' '${enable_globus:-yes}'		# "yes" or "no"
#  add_enable globus_sdk 'globus_sdk' 'Globus SDK library' '${enable_globus_sdk:-yes}'		# "yes" or "no"
  add_enable gsoap 'gsoap' 'gSOAP library' '${enable_gsoap:-yes}'				# "yes" or "no"
  add_enable cgsi_plugin 'cgsi-plugin' 'CGSI plugin' '${enable_cgsi_plugin:-yes}'		# "yes" or "no"
  add_enable gfal 'gfal' 'GFAL library' '${enable_gfal:-no}'					# "yes" or "no"
}

l_set_package() {
  GLOBUS_FLAVOUR=${GLOBUS_FLAVOUR:-gcc32dbgpthr}
  LIB_GLOBUS=${LIB_GLOBUS:-"-lglobus_gssapi_gsi_$GLOBUS_FLAVOUR -lglobus_gss_assist_$GLOBUS_FLAVOUR"}
  LIB_GLOBUS_SDK=${LIB_GLOBUS}
  LIB_GSOAP=${LIB_GSOAP:-"-lgsoap"}
  LIB_CGSI_PLUGIN=${LIB_CGSI_PLUGIN:-"-lcgsi_plugin_gsoap_2.7"}
  l_add_opts
  l_add_withdirs
  l_add_statics
  l_add_enables
}

l_set_opt_deps() {
  # option dependencies
  if ! test x${enable_gsoap} = xyes || ! test x${enable_srm2} = xyes ; then
    enable_gsoap=no
    enable_srm2=no
  fi
}

l_have_checks() {
  have_globus
#  have_globus_sdk
  have_gsoap
  have_cgsi_plugin
}

l_have_checks_no_cross() {
  $_NOP
}

l_cmdline_opts() {
  $_NOP
}

l_write_settings() {
  l_CONFIG_MAK
  l_CONFIG_H
  l_RPMBUILD
}

l_summary() {
#Globus SDK lib:              ${have_globus_sdk:-no}/${enable_globus_sdk:-no}
  cat >&2 <<EOF

SRM2 support:                ${enable_srm2:-no}
Globus essentials lib:       ${have_globus:-no}/${enable_globus:-no}, static: ${static_globus:-no}
gSOAP lib:                   ${have_gsoap:-no}/${enable_gsoap:-no}, static: ${static_gsoap:-no}
CGSI plugin:                 ${have_cgsi_plugin:-no}/${enable_cgsi_plugin:-no}, static: ${static_cgsi_plugin:-no}
GFAL lib:                    ${have_gfal:-no}/${enable_gfal:-no}
EOF
}

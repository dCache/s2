#
# Defines
#
%include .rpmdef

Summary:	S2 test client
Name:		%{_name}
Version:	%{_version}
Release:	%{_release}
Copyright:	BSD License
Group:		Development/Tools
URL:		http://s-2.sourceforge.net
Source:		%{name}-%{version}.tar.gz
Prefix:		%{_prefix}
BuildRoot:	%{_tmppath}/%{name}-%{version}-buildroot
BuildArch:	i386
  

#
# Requires
#
%include .rpmreq


#
# Description
#
%description
A simple general-purpose test client with the SRM2 protocol support.


#
# Packages
#
#%package devel
#Summary: development tools for %name
#Group: Development/Libraries
##Requires: %name = %version

#%package doc
#Summary: documentation for %name
#Group: Development/Libraries
#Requires: %name = %version

#%description devel
#The %name-devel package contains an 4 file
#needed for development with %name.

#%description doc
#The %name-doc package contains the %name documentation


#
# Prep
#
%prep
%setup -q -n %{name}-%{version}


#
# Build
#
%build
CFLAGS="-mcpu=i386 $CFLAGS"
make


#
# Install
#
%install
make install prefix=%{buildroot}/

%clean

#
# Script sections
#
%post
/sbin/ldconfig

%preun

%postun
/sbin/ldconfig
rmdir %{_bindir} 2>/dev/null
rmdir %{_includedir} 2>/dev/null
rmdir %{_libdir} 2>/dev/null
rmdir %{_docdir} 2>/dev/null


### Files ############################################################
%files
%include .rpmfiles


%changelog
* Wed Feb 12 2006 Jiri Mencak
- 0.02 cleanup

* Wed Nov 29 2005 Jiri Mencak
- 0.01

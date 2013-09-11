%{!?_release: %define _release none}
Name:		iofwd
Version:	1.1.0
Release:	%{_release}%{?dist}
Summary:	IOFWD server and client libraries.
Group:		Productivity/Networking/Iofsl
License:	BSD
Source:		%{name}-%{version}.tar.gz
Prefix:         /usr
BuildRoot:	%{_buildrootdir}

BuildRequires: orangefs
Requires:   orangefs	

%description
IOFWD server and client libraries.

%prep
%setup -q

%build
./configure CFLAGS=-fPIC --with-boost --with-boost-libdir=/usr/lib64 --without-mpi --prefix=%{_prefix} --libdir=%{_libdir}
%{__make} %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
%{__make} install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc defaultconfig.cf
%{_libdir}/*
%{_libdir}/pkgconfig/iofslrouter.pc
%{_libdir}/pkgconfig/iofsl.pc
%{_bindir}/zoidfsfuse
%{_bindir}/iofwd
%{_includedir}/*

%changelog
* Tue Jun 04 2013 Zhang Jingwang <jingwang.zhang@emc.com> - 1.1.0-1
- Rewrite the spec file to get rid of rpmlint warning messages.

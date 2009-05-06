%define ps3 0

%if %{ps3}
%define _libdir /usr/lib
%endif
%define libdir /usr/lib

Summary: ActiveMQ CPP Library
Name: activemq-cpp
Version: 2.1.3
Release: 2.fc7
License: Apache License
Group: Development/Tools/Building
URL: http://activemq.apache.org/activemq-c-clients.html
Source0: %{name}-%{version}-src.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: e2fsprogs-devel  >= 1.38
BuildRequires: autoconf         >= 2.59
BuildRequires: automake         >= 1.9.6
BuildRequires: libtool          >= 1.5.22
BuildRequires: cppunit-devel   >= 1.12

%description

#%package devel
#Group:  Development/Tools/Building
#Summary: ActiveMQ CPP Library
#%description devel
#ActiveMQ CPP Library

#%package shared
#Group:  Development/Tools/Building
#Summary: ActiveMQ CPP Library
#%description shared
#ActiveMQ CPP Shared Library

%prep
%setup -q -n %{name}-%{version}-src

%build
./autogen.sh
#./configure --prefix=/usr --includedir=
./configure --prefix=/usr --disable-shared
make
make doxygen-run
make check

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install
rm -f $RPM_BUILD_ROOT%{_bindir}/example
mv $RPM_BUILD_ROOT%{_includedir}/activemq-cpp-%{version} $RPM_BUILD_ROOT%{_includedir}/activemq-cpp
(cd $RPM_BUILD_ROOT/%{_includedir}; ln -s activemq-cpp activemq-cpp-2.1.3)
#(cd $RPM_BUILD_ROOT/%{_libdir}; ln -s libactivemq-cpp.so libactivemq-cpp-2.1.3.so)
(cd $RPM_BUILD_ROOT/%{libdir}; ln -s libactivemq-cpp.a libactivemq-cpp-2.1.3.a)
sed -e 's%Version:.*%Version: 2.1.3%' $RPM_BUILD_ROOT/%{libdir}/pkgconfig/activemq-cpp.pc > dummy
mv dummy $RPM_BUILD_ROOT/%{libdir}/pkgconfig/activemq-cpp.pc

%clean
#rm -rf $RPM_BUILD_ROOT
echo $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%doc
#/usr/bin/example
%{_bindir}/simple_async_consumer
%{_bindir}/simple_producer
%{_bindir}/activemqcpp-config
%{_includedir}/activemq-cpp*
%{libdir}/libactivemq-cpp*
%{libdir}/pkgconfig/activemq-cpp.pc

#%files devel
#%defattr(-,root,root,-)
#%doc
#%{_includedir}/activemq-cpp
#%{_libdir}/libactivemq-cpp.a
#%{_libdir}/pkgconfig/activemq-cpp.pc
#%files shared
#%{_libdir}/libactivemq-cpp.la
#%{_libdir}/libactivemq-cpp*so*

%changelog
* Wed Oct 10 2007 Kai Krueger <krueger@itwm.fraunhofer.de> - cpp-1
- Initial build.


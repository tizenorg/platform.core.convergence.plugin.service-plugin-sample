Name:       org.tizen.service-plugin-sample
Summary:    plugin sample
Version:    0.1.6
Release:    1
Group:      Test
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(service-adaptor)
BuildRequires:  cmake
BuildRequires:  edje-bin
#BuildRequires:  service-adaptor-devel

%description
plugin sample

%package devel
Summary:    Development files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
%description devel
Development files for %{name}

%prep
%setup -q

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif

cmake . -DCMAKE_INSTALL_PREFIX=/usr

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post
cp /opt/share/applications/org.tizen.service-plugin-sample.desktop /opt/share/
cp -f /opt/share/org.tizen.service-plugin-sample.desktop /opt/share/applications/

%files
%manifest org.tizen.service-plugin-sample.manifest
/opt/usr/apps/org.tizen.service-plugin-sample/*
/opt/share/packages/org.tizen.service-plugin-sample.xml
/opt/share/icons/default/small/org.tizen.service-plugin-sample.png
/etc/smack/accesses2.d/org.tizen.service-plugin-sample.rule

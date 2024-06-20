Name: asap
Version: 5.2.0
Release: 1
Summary: Player of Atari 8-bit music
License: GPLv2+
Group: Applications/Multimedia
Source: http://prdownloads.sourceforge.net/asap/asap-%{version}.tar.gz
URL: http://asap.sourceforge.net/
BuildRequires: gcc
BuildRoot: %{_tmppath}/asap-root

%description
ASAP is a player of Atari 8-bit music for modern computers.
It emulates the POKEY sound chip and the 6502 processor.
ASAP supports the following file formats:
SAP, CMC, CM3, CMR, CMS, DMC, DLT, MPT, MPD, RMT, TMC, TM8, TM2, FC.

%package devel
Summary: Development library providing Atari 8-bit music emulation
Group: Development/Libraries

%description devel
These are the files needed for compiling programs that use libasap.

%package vlc
Summary: ASAP plugin for VLC
Group: Applications/Multimedia
Requires: vlc
BuildRequires: vlc-devel

%description vlc
Provides playback of Atari 8-bit music in VLC.
Supports the following file formats: SAP, RMT, FC.

%package xmms2
Summary: ASAP plugin for XMMS2
Group: Applications/Multimedia
Requires: xmms2
BuildRequires: xmms2-devel

%description xmms2
Provides playback of Atari 8-bit music (SAP format) in XMMS2.

%global debug_package %{nil}

%prep
%setup -q

%build
make asapconv libasap.a asap-vlc asap-xmms2

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT prefix=%{_prefix} libdir=%{_libdir} install install-vlc install-xmms2

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/asapconv

%files devel
%defattr(-,root,root)
%{_includedir}/asap.h
%{_libdir}/libasap.a

%files vlc
%defattr(-,root,root)
%{_libdir}/vlc/plugins/demux/libasap_plugin.so

%files xmms2
%defattr(-,root,root)
%{_libdir}/xmms2/libxmms_asap.so

%changelog
* Wed Dec 8 2021 Piotr Fusik <fox@scene.pl>
- 5.2.0-1

* Tue Nov 30 2021 Piotr Fusik <fox@scene.pl>
- Added the XMMS2 subpackage
- Removed the XMMS subpackage

* Fri Jul 9 2021 Piotr Fusik <fox@scene.pl>
- 5.1.0-1

* Sun Jan 19 2020 Piotr Fusik <fox@scene.pl>
- 5.0.1-1

* Thu Nov 21 2019 Piotr Fusik <fox@scene.pl>
- 5.0.0-1

* Thu Jan 10 2019 Piotr Fusik <fox@scene.pl>
- 4.0.0-1

* Sat Aug 12 2017 Piotr Fusik <fox@scene.pl>
- Discontinued GStreamer

* Mon Jun 23 2014 Piotr Fusik <fox@scene.pl>
- 3.2.0-1

* Wed Jan 15 2014 Piotr Fusik <fox@scene.pl>
- 3.1.6-1

* Fri Aug 16 2013 Piotr Fusik <fox@scene.pl>
- 3.1.5-1
- Corrected descriptions of GStreamer and VLC plugins - they don't support all the formats

* Mon Apr 29 2013 Piotr Fusik <fox@scene.pl>
- 3.1.4-1
- lib64 compatibility
- Removed the Audacious subpackage

* Tue Dec 4 2012 Piotr Fusik <fox@scene.pl>
- 3.1.3-1
- Added subpackages with GStreamer and VLC plugins

* Mon Jun 25 2012 Piotr Fusik <fox@scene.pl>
- 3.1.2-1

* Wed Oct 26 2011 Piotr Fusik <fox@scene.pl>
- 3.1.1-1

* Sat Sep 24 2011 Piotr Fusik <fox@scene.pl>
- 3.1.0-1

* Fri Jul 15 2011 Piotr Fusik <fox@scene.pl>
- 3.0.1-1

* Thu May 19 2011 Piotr Fusik <fox@scene.pl>
- 3.0.0-1
- Added subpackages with Audacious and XMMS plugins

* Wed Nov 3 2010 Piotr Fusik <fox@scene.pl>
- 2.1.2-1
- Initial packaging

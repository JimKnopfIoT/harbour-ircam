# Neutral packaging metadata — no personal identifiers.
Name:       harbour-ircam
Summary:    Thermal camera viewer for the InfiRay P2 Pro
Version:    0.1.1
Release:    1
# ANONYMITY: neutral build host so built RPMs carry no real hostname/domain.
%define _buildhost reproducible-builder
License:    GPL-3.0-or-later
URL:        https://github.com/JimKnopfIoT/harbour-ircam
Source0:    %{name}-%{version}.tar.bz2
Vendor:     harbour-ircam contributors
Packager:   harbour-ircam contributors

Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   nemo-qml-plugin-configuration-qt5
Requires:   qt5-qtdeclarative-import-positioning
# vp8enc + webmmux for video recording live in gstreamer plugins-good;
# gstreamer core arrives as a soname dependency. gstreamer dlopens the
# encoder at runtime, so rpm cannot infer this — state it explicitly.
Requires:   gstreamer1.0-plugins-good
BuildRequires: pkgconfig(sailfishapp) >= 1.0.2
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(gstreamer-1.0)
BuildRequires: pkgconfig(gstreamer-app-1.0)
BuildRequires: qt5-qttools-linguist
BuildRequires: desktop-file-utils

%description
Live view, temperature measurement and snapshots for the InfiRay P2 Pro USB-C
thermal camera. The camera streams over V4L2; IrCam colours the picture with a
choice of palettes, decodes the per-pixel temperature plane, shows hottest and
coldest spot, a centre spot and up to six tap-to-measure points, a colour bar,
and writes snapshots (optionally with the raw temperature grid as CSV) into
~/Pictures/IrCam. Everything runs on the device: no network access, no
telemetry.

%prep
%setup -q

%build
%qmake5
%make_build

%install
%qmake5_install

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png

%changelog
* Sun Aug 30 2026 harbour-ircam contributors 0.1.1-1
- The controls keep clear of the camera cutout. On a phone with a notch
  the top row of buttons and the live status line were cut through by it,
  so the icons were only half visible; they now start below it. The
  picture itself still fills the screen. On a phone without a cutout
  nothing moves.

* Wed Aug 19 2026 harbour-ircam contributors 0.1.0-1
- First release: live view, palettes, min/max/centre/tap measurements,
  colour bar, snapshots with optional CSV temperature grid, gallery.

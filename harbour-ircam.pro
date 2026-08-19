# harbour-ircam — SailfishOS qmake project
#
# A thermal camera viewer for the InfiRay P2 Pro (UVC 0bda:5830). The camera
# streams a 256x384 YUYV picture over V4L2 whose lower half is a 16-bit
# temperature plane; see src/camera/thermalcamera.h for the decode.
#
# Build:  mb2 --target SailfishOS-<version>-aarch64 build

TARGET = harbour-ircam

CONFIG += sailfishapp sailfishapp_i18n c++11

QT += quick

# The capture loop runs in a std::thread.
LIBS += -lpthread

# Video recording pushes composed frames through an appsrc into a VP8/WebM
# pipeline. gstreamer-app-1.0 provides the appsrc; do NOT add link_pkgconfig
# here (sailfishapp.prf adds it and appends `sailfishapp` to PKGCONFIG itself).
PKGCONFIG += gstreamer-1.0 gstreamer-app-1.0

# Bilingual: English source strings + German. libsailfishapp auto-loads the .qm
# matching the device locale.
TRANSLATIONS += translations/harbour-ircam-de.ts
lupdate_only {
    SOURCES += qml/*.qml qml/cover/*.qml qml/pages/*.qml qml/components/*.qml
}

# Installed to /usr/share/icons/hicolor/<size>/apps/ (see sailfishapp.prf).
SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

INCLUDEPATH += \
    src/app \
    src/camera

HEADERS += \
    src/app/capturestore.h \
    src/camera/cameraprofile.h \
    src/camera/palettes.h \
    src/camera/snapshotcomposer.h \
    src/camera/thermalcamera.h \
    src/camera/videorecorder.h \
    src/camera/thermalframeitem.h

SOURCES += \
    src/app/harbour-ircam.cpp \
    src/app/capturestore.cpp \
    src/camera/cameraprofile.cpp \
    src/camera/palettes.cpp \
    src/camera/snapshotcomposer.cpp \
    src/camera/thermalcamera.cpp \
    src/camera/videorecorder.cpp \
    src/camera/thermalframeitem.cpp

coverimages.files = qml/images/cover-logo.png
coverimages.path  = /usr/share/$${TARGET}/qml/images
INSTALLS += coverimages

OTHER_FILES += \
    qml/harbour-ircam.qml \
    qml/cover/CoverPage.qml \
    qml/pages/ViewfinderPage.qml \
    qml/pages/GalleryPage.qml \
    qml/pages/CaptureViewPage.qml \
    qml/pages/RenameDialog.qml \
    qml/pages/SettingsPage.qml \
    qml/pages/AboutPage.qml \
    qml/pages/CamerasPage.qml \
    qml/components/ShutterButton.qml \
    qml/components/StatusOverlay.qml \
    qml/components/MeasurementOverlay.qml \
    qml/components/CameraDbView.qml \
    qml/components/ToggleButton.qml \
    harbour-ircam.desktop \
    rpm/harbour-ircam.spec

DISTFILES += README.md

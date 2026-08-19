// harbour-ircam — InfiRay P2 Pro thermal viewer for Sailfish OS. Entry point. GPLv3+. (C) 2026 JimKnopfIoT.
#include <QtQuick>
#include <sailfishapp.h>

#include "capturestore.h"
#include "thermalcamera.h"
#include "thermalframeitem.h"
#include "videorecorder.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));

    // Wraps gst_init() — required before any VideoRecorder is built.
    VideoRecorder::initGStreamer(&argc, &argv);

    const char *uri = "harbour.ircam";
    qmlRegisterType<ThermalCamera>(uri, 1, 0, "ThermalCamera");
    qmlRegisterType<ThermalFrameItem>(uri, 1, 0, "ThermalView");
    qmlRegisterType<CaptureStore>(uri, 1, 0, "CaptureStore");
    qmlRegisterType<VideoRecorder>(uri, 1, 0, "VideoRecorder");

    QScopedPointer<QQuickView> view(SailfishApp::createView());
    view->setSource(SailfishApp::pathToMainQml());
    view->show();

    return app->exec();
}

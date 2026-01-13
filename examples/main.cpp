#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QQuickWindow>

int main(int argc, char* argv[]) {
    // Enable MSAA for antialiased edges
    QSurfaceFormat format;
    format.setSamples(4);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Add import path for the plugin
    engine.addImportPath(QStringLiteral("qrc:/"));

    // Load the QML file
    const QUrl url(QStringLiteral("qrc:/qt/qml/ShapeGalleryApp/examples/ShapeGallery.qml"));

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (obj == nullptr && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}

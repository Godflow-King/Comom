#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "showimage.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    ShowImage *CodeImage = new ShowImage();
    engine.rootContext()->setContextProperty("CodeImage",CodeImage);
    engine.addImageProvider(QLatin1String("CodeImg"), CodeImage->m_pImgProvider);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;


    return app.exec();
}

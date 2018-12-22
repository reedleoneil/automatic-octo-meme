#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtWebView/QtWebView>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    QtWebView::initialize();

    QString tmploc = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir tmpdir(tmploc + "/fireapp");

    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString tmpfile;
        tmpfile = it.next();
        if (QFileInfo(tmpfile).isFile()) {
            QFileInfo file = QFileInfo(tmpdir.absolutePath() + tmpfile.right(tmpfile.size()-1));
            file.dir().mkpath("."); // create full path if necessary
            QFile::remove(file.absoluteFilePath()); // remove previous file to make sure we have the latest version
            QFile::copy(tmpfile, file.absoluteFilePath());
        }
    }

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    engine.rootContext()->setContextProperty(QStringLiteral("path"), "file://" + QFileInfo(tmpdir.absolutePath() + "/index.html").absoluteFilePath());
    qDebug() << QFileInfo(tmpdir.absolutePath() + "/index.html").absoluteFilePath();
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

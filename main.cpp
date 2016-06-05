#include <QApplication>

#include <QtConcurrent/QtConcurrentRun>

#include "mainwindow.h"
#include "feedimagecache.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationVersion("0.1");
#ifdef Q_OS_WIN
    a.setFont(QFont("Segoe UI Emoji", 10));
#endif

    QtConcurrent::run(FeedImageCache::cleanCache);
    qsrand(time(nullptr));

    MainWindow w;
    w.show();

    return a.exec();
}

#include <QApplication>
#include <QtConcurrent/QtConcurrentRun>
#include <ctime>

#include "mobilesyncwidget.h"
#include "feedimagecache.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationVersion("0.1");
    a.setApplicationName(QStringLiteral("HornAppDesktop"));
    a.setOrganizationName(QStringLiteral("kambala"));
    a.setOrganizationDomain(QStringLiteral("com.kambala"));
#ifdef Q_OS_WIN
    a.setFont(QFont("Segoe UI Symbol", 10));
#endif

    QtConcurrent::run(FeedImageCache::cleanCache);
    qsrand(time(nullptr));
    MobileSyncWidget::syncOrStartApp();

    return a.exec();
}

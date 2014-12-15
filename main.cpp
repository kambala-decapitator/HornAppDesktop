#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationVersion("0.1");
#ifdef Q_OS_WIN
    a.setFont(QFont("Segoe UI Emoji", 10));
#endif

    Widget w;
    w.show();

    return a.exec();
}

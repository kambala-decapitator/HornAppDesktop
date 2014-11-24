#include "widget.h"
#include "requestmanager.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    RequestManager::instance();

    Widget w;
    w.show();

    return a.exec();
}

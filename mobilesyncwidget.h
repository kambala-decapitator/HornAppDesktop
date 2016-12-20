#ifndef MOBILESYNCWIDGET_H
#define MOBILESYNCWIDGET_H

#include <QWidget>

class MobileSyncWidget : public QWidget
{
    Q_OBJECT

public:
    static void syncOrStartApp();

private:
    static void startApp(const QString &userId, const QString &token);

private:
    explicit MobileSyncWidget(QWidget *parent = 0);
    virtual ~MobileSyncWidget() {}
};

#endif // MOBILESYNCWIDGET_H

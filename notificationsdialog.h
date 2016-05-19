#ifndef NOTIFICATIONSDIALOG_H
#define NOTIFICATIONSDIALOG_H

#include <QDialog>

#include "feeditem.h"

namespace Ui {
class NotificationsDialog;
}

class NotificationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NotificationsDialog(QWidget *parent = 0);
    ~NotificationsDialog();

private slots:
    void requestNotifications();

private:
    Ui::NotificationsDialog *ui;
    TextItemList _feed;
};

#endif // NOTIFICATIONSDIALOG_H

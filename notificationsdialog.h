#ifndef NOTIFICATIONSDIALOG_H
#define NOTIFICATIONSDIALOG_H

#include <QDialog>

#include "feeditem.h"

namespace Ui {
class NotificationsDialog;
}
class QListWidgetItem;

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

    void setReadStateForListItem(bool isRead, QListWidgetItem *item);
#ifdef Q_OS_MAC
    void updateMacBadge(int value);

    void displaySystemNotification(const QString &text, quint32 notificationId);
    void removeSystemNotificationWithId(quint32 notificationId);
    void removeSystemNotifications();
#endif
};

#endif // NOTIFICATIONSDIALOG_H

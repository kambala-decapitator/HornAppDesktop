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

    void openPostFromNotificationId(decltype(NotificationItem::id) notificationId, bool openPost);

private slots:
    void requestNotifications();

private:
    Ui::NotificationsDialog *ui;
    TextItemList _feed;

    void openPostFromNotificationWithIndex(int row, bool openPost = true);
    void setReadStateForListItem(bool isRead, QListWidgetItem *item);

#ifdef Q_OS_MAC
    void updateMacBadge(int value);

    void displaySystemNotification(const QString &text, const QString &dateTimeStr, decltype(NotificationItem::postId) postId, decltype(NotificationItem::id) notificationId);
    void removeSystemNotificationWithId(quint32 notificationId);
    void removeSystemNotifications();
#endif
};

#endif // NOTIFICATIONSDIALOG_H

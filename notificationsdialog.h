#ifndef NOTIFICATIONSDIALOG_H
#define NOTIFICATIONSDIALOG_H

#include <QDialog>
#include "feeditem.h"

namespace Ui {
class NotificationsDialog;
}
class QListWidgetItem;
#ifdef Q_OS_WIN
class QWinTaskbarButton;
#endif

class NotificationsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NotificationsDialog(QWidget *parent = 0);
    virtual ~NotificationsDialog();

    void openPostFromNotificationId(decltype(NotificationItem::id) notificationId, bool openPost);
#ifdef Q_OS_WIN
    void setMainWindowHandle(QWindow *mainWindowHandle) { _mainWindowHandle = mainWindowHandle; }
#endif

private slots:
    void requestNotifications();

private:
    Ui::NotificationsDialog *ui;
    TextItemList _feed;
    int _unreadCount = 0;
#ifdef Q_OS_WIN
    QWindow *_mainWindowHandle = nullptr;
    QWinTaskbarButton *_taskbarButton = nullptr;
#endif

    void openPostFromNotificationWithIndex(int row, bool openPost = true);
    void setReadStateForListItem(bool isRead, QListWidgetItem *item);
    void updateAppIconWithUnreadCount(int value);

#ifdef Q_OS_MACOS
    void displaySystemNotification(const QString &text, const QString &dateTimeStr, decltype(NotificationItem::postId) postId, decltype(NotificationItem::id) notificationId);
    void removeSystemNotificationWithId(quint32 notificationId);
    void removeSystemNotifications();
#endif
};

#endif // NOTIFICATIONSDIALOG_H

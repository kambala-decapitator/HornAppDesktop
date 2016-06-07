#include "notificationsdialog.h"
#include "ui_notificationsdialog.h"
#include "requestmanager.h"
#include "commentsdialog.h"

#include <QMessageBox>
#include <QTimer>

#ifdef Q_OS_MAC
#include <QtMac>
#endif

NotificationsDialog::NotificationsDialog(QWidget *parent) : QDialog(parent, Qt::Tool | Qt::WindowStaysOnTopHint), ui(new Ui::NotificationsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);

    requestNotifications();

    auto timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(requestNotifications()));
    timer->start(30 * 1000);

    connect(ui->listWidget, &QListWidget::doubleClicked, [this](const QModelIndex &index){
        openPostFromNotificationWithIndex(index.row());
    });

    connect(ui->markAllReadButton, &QPushButton::clicked, [this]{
        QList<decltype(NotificationItem::id)> unreadIds;
        for (int i = 0; i < _feed.size(); ++i)
        {
            auto notification = static_cast<NotificationItem *>(_feed.at(i));
            if (!notification->isRead)
                unreadIds << notification->id;
            notification->isRead = true;

            setReadStateForListItem(true, ui->listWidget->item(i));
        }
        RequestManager::instance().markNotificationsRead(unreadIds);

#ifdef Q_OS_MAC
        updateMacBadge(0);
        removeSystemNotifications();
#endif
    });
}

NotificationsDialog::~NotificationsDialog()
{
    delete ui;
    qDeleteAll(_feed);
}

void NotificationsDialog::openPostFromNotificationId(decltype(NotificationItem::id) notificationId)
{
    for (int i = 0; i < _feed.size(); ++i)
    {
        if (_feed.at(i)->id == notificationId)
        {
            openPostFromNotificationWithIndex(i);
            return;
        }
    }
}

void NotificationsDialog::requestNotifications()
{
    RequestManager::instance().requestNotifications([this](const TextItemList &feed){
        if (feed.isEmpty())
            return;

        bool hasNew = _feed.isEmpty() ? true : _feed.first()->id != feed.first()->id;
        qDeleteAll(_feed);
        _feed = feed;

        ui->listWidget->scrollToTop();
        ui->listWidget->clear();

        int unread = 0;
        for (auto item : feed)
        {
            auto notification = static_cast<NotificationItem *>(item);
            auto lwItem = new QListWidgetItem(ui->listWidget);
            lwItem->setToolTip(QString::number(notification->postId));
            setReadStateForListItem(notification->isRead, lwItem);

            QString text;
            if (notification->type == QLatin1String("NOTICE_NEW_HORN_COMMENT_REPLY") || notification->type == QLatin1String("NOTICE_NEW_HORN_COMMENT"))
                text = notification->message;
            else if (notification->type == QLatin1String("NOTICE_NEW_HORN_COMMENT_VOTE"))
                text = tr("comment vote");
            else if (notification->type == QLatin1String("NOTICE_DEL_HORN_COMMENT_VOTE"))
                text = tr("comment vote removed");
            else if (notification->type == QLatin1String("NOTICE_NEW_HORN_VOTE"))
                text = tr("post vote");
            else if (notification->type == QLatin1String("NOTICE_DEL_HORN_VOTE"))
                text = tr("post vote removed");
            else
                text = notification->type + " - " + notification->message;
            lwItem->setText(text);

            if (!notification->isRead)
            {
                ++unread;
#ifdef Q_OS_MAC
                displaySystemNotification(text, notification->id);
#endif
            }
        }

        if (unread && hasNew)
            qApp->alert(this);

#ifdef Q_OS_MAC
        updateMacBadge(unread);
#endif
    });
}

void NotificationsDialog::openPostFromNotificationWithIndex(int row)
{
    auto notification = static_cast<NotificationItem *>(_feed.at(row));
    RequestManager::instance().requestPostWithId(notification->postId, [notification, this](FeedItem *feedItem){
        if (!feedItem)
        {
            QMessageBox::critical(this, QString(), tr("Error opening post"));
            return;
        }
        RequestManager::instance().requestComments(feedItem->id, [notification, feedItem, this](const TextItemList &comments) {
            CommentsDialog *w = new CommentsDialog(feedItem, comments, true, QSet<quint32>({notification->commentId1, notification->commentId2}), parentWidget(), Qt::Window);
            w->show();
        });
    });

    if (!notification->isRead)
    {
        RequestManager::instance().markNotificationsRead({notification->id});

        notification->isRead = true;
        setReadStateForListItem(true, ui->listWidget->item(row));

#ifdef Q_OS_MAC
        updateMacBadge(QtMac::badgeLabelText().toInt() - 1);
        removeSystemNotificationWithId(notification->id);
#endif
    }
}

void NotificationsDialog::setReadStateForListItem(bool isRead, QListWidgetItem *item)
{
    item->setTextColor(isRead ? Qt::gray : Qt::black);
}

#ifdef Q_OS_MAC
void NotificationsDialog::updateMacBadge(int value)
{
    QtMac::setBadgeLabelText(value > 0 ? QString::number(value) : QString());
}
#endif

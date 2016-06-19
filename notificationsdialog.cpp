#include "notificationsdialog.h"
#include "ui_notificationsdialog.h"
#include "requestmanager.h"
#include "commentsdialog.h"

#include <QMessageBox>

#include <QDateTime>
#include <QTimer>

#ifdef Q_OS_MAC
#include <QtMac>
#endif

static const int RefreshSecs = 30;

NotificationsDialog::NotificationsDialog(QWidget *parent) : QDialog(parent, Qt::Tool | Qt::WindowStaysOnTopHint), ui(new Ui::NotificationsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);

    requestNotifications();

    auto timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(requestNotifications()));
    timer->start(RefreshSecs * 1000);

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

void NotificationsDialog::openPostFromNotificationId(decltype(NotificationItem::id) notificationId, bool openPost)
{
    for (int i = 0; i < _feed.size(); ++i)
    {
        if (_feed.at(i)->id == notificationId)
        {
            openPostFromNotificationWithIndex(i, openPost);
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
            setReadStateForListItem(notification->isRead, lwItem);

            // TODO: copy-paste
            auto dateTime = QDateTime::fromTime_t(notification->timestamp);
            auto today = QDate::currentDate(), date = dateTime.date();
            QLatin1String dateFormat;
            if (!date.daysTo(today))
                dateFormat = QLatin1String("h:mm");
            else if (date.year() == today.year())
                dateFormat = QLatin1String("d.M h:mm");
            else
                dateFormat = QLatin1String("d.M.yy h:mm");
            auto dateTimeStr = dateTime.toString(dateFormat);
            lwItem->setToolTip(QString("%1\n%2").arg(dateTimeStr).arg(notification->postId));

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
                displaySystemNotification(text, dateTimeStr, notification->postId, notification->id);
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

void NotificationsDialog::openPostFromNotificationWithIndex(int row, bool openPost)
{
    auto notification = static_cast<NotificationItem *>(_feed.at(row));
    if (openPost)
        CommentsDialog::instance().showComments(notification->postId, nullptr, {notification->commentId1, notification->commentId2});

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

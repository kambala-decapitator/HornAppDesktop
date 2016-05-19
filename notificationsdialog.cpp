#include "notificationsdialog.h"
#include "ui_notificationsdialog.h"
#include "requestmanager.h"

#include <QTimer>

NotificationsDialog::NotificationsDialog(QWidget *parent) : QDialog(parent, Qt::Tool), ui(new Ui::NotificationsDialog)
{
    ui->setupUi(this);

    requestNotifications();

    auto timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(requestNotifications()));
    timer->start(60 * 1000);

    connect(ui->listWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item){
        // [id1,id2,...]
        // PATCH /request/v1/UserNotification/%5B notification_id %5D?token=...
    });
}

NotificationsDialog::~NotificationsDialog()
{
    delete ui;
}

void NotificationsDialog::requestNotifications()
{
    RequestManager::instance().requestNotifications([this](const TextItemList &feed){
        ui->listWidget->clear();
        for (const auto &item : feed)
        {
            auto notification = static_cast<NotificationItem *>(item);
            auto lwItem = new QListWidgetItem(ui->listWidget);
            lwItem->setTextColor(notification->isRead ? Qt::gray : Qt::black);
            if (notification->type == QLatin1String("NOTICE_NEW_HORN_COMMENT_REPLY"))
                lwItem->setText(notification->message);
            else if (notification->type == QLatin1String("NOTICE_NEW_HORN_COMMENT_VOTE"))
                lwItem->setText(tr("vote"));
            else
                lwItem->setText(notification->type + " - " + notification->message);
        }
    });
}

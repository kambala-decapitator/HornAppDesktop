#include "notificationsdialog.h"

#import <Foundation/Foundation.h>

void NotificationsDialog::displaySystemNotification(const QString &text, quint32 notificationId)
{
    if (![NSUserNotificationCenter class])
        return;

    auto n = [NSUserNotification new];
    n.informativeText = text.toNSString();
    n.identifier = @(notificationId).description;
    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:n];
}

void NotificationsDialog::removeSystemNotificationWithId(quint32 notificationId)
{
    if (![NSUserNotificationCenter class])
        return;

    auto notificationIdStr = @(notificationId).description;
    for (NSUserNotification *n in [NSUserNotificationCenter defaultUserNotificationCenter].deliveredNotifications)
    {
        if ([n.identifier isEqualToString:notificationIdStr])
        {
            [[NSUserNotificationCenter defaultUserNotificationCenter] removeDeliveredNotification:n];
            break;
        }
    }
}

void NotificationsDialog::removeSystemNotifications()
{
    if ([NSUserNotificationCenter class])
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
}

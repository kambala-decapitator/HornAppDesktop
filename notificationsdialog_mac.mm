#include "notificationsdialog.h"

#import <Foundation/Foundation.h>


@interface UserNotificationCenterDelegate : NSObject <NSUserNotificationCenterDelegate>
+ (instancetype)sharedInstance;
@property (nonatomic, unsafe_unretained) NotificationsDialog *dialog;
@end

@implementation UserNotificationCenterDelegate

+ (instancetype)sharedInstance
{
    static id instance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{ instance = [self new]; });
    return instance;
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *__unused)center shouldPresentNotification:(NSUserNotification *__unused)notification { return YES; }

- (void)userNotificationCenter:(NSUserNotificationCenter *__unused)center didActivateNotification:(NSUserNotification *)notification
{
    self.dialog->openPostFromNotificationId(notification.identifier.integerValue, notification.activationType != NSUserNotificationActivationTypeActionButtonClicked);
}

@end


bool systemNotificationsAvailable()
{
    static bool b = [NSUserNotificationCenter class] != nil;
    return b;
}

void NotificationsDialog::displaySystemNotification(const QString &text, const QString &dateTimeStr, decltype(NotificationItem::postId) postId, decltype(NotificationItem::id) notificationId)
{
    if (!systemNotificationsAvailable())
        return;

    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        auto delegate = [UserNotificationCenterDelegate sharedInstance];
        delegate.dialog = this;
        [NSUserNotificationCenter defaultUserNotificationCenter].delegate = delegate;
    });

    auto n = [NSUserNotification new];
    n.subtitle = [NSString stringWithFormat:@"%@, %u", dateTimeStr.toNSString(), postId];
    n.informativeText = text.toNSString();
    n.identifier = @(notificationId).description;
    n.actionButtonTitle = tr("Mark Read").toNSString(); // if app is not signed, then Alert style must be set in System Preferences for this to work
    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:n];
}

void NotificationsDialog::removeSystemNotificationWithId(quint32 notificationId)
{
    if (!systemNotificationsAvailable())
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
    if (systemNotificationsAvailable())
        [[NSUserNotificationCenter defaultUserNotificationCenter] removeAllDeliveredNotifications];
}

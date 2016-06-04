#include "feedwidget.h"

#import <AppKit/AppKit.h>

void FeedWidget::showLocation(double latitude, double longitude)
{
    static auto const pinTitleEncoded = @"%F0%9F%92%A9";
    auto params = [NSString stringWithFormat:@"t=m&z=13&q=%@&ll=%lf,%lf", pinTitleEncoded, latitude, longitude];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[@"http://maps.apple.com/?" stringByAppendingString:params]]];
}

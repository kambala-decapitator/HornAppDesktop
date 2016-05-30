#include "feedwidget.h"

#import <AppKit/AppKit.h>

void FeedWidget::showLocation(double latitude, double longitude)
{
    auto params = [NSString stringWithFormat:@"t=m&z=13&q=%%F0%%9F%%92%%A9&ll=%lf,%lf", latitude, longitude];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[@"http://maps.apple.com/?" stringByAppendingString:params]]];
}

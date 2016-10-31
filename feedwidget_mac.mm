#include "feedwidget.h"

#import <AppKit/AppKit.h>
#import <Quartz/Quartz.h>


@interface QuickLookHandler : NSResponder <QLPreviewPanelDataSource>
@property (nonatomic, copy) NSURL *imageFileUrl;
@end

@implementation QuickLookHandler

- (BOOL)acceptsFirstResponder { return YES; }

#pragma mark - QLPreviewPanelController

- (BOOL)acceptsPreviewPanelControl:(QLPreviewPanel *__unused)panel { return YES; }
- (void)beginPreviewPanelControl:(QLPreviewPanel *)panel { panel.dataSource = self; }
- (void)endPreviewPanelControl:(QLPreviewPanel *)panel   { panel.dataSource = nil; }

#pragma mark - QLPreviewPanelDataSource

- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *__unused)panel { return 1; }
- (id<QLPreviewItem>)previewPanel:(QLPreviewPanel *__unused)panel previewItemAtIndex:(NSInteger __unused)index { return self.imageFileUrl; }

@end


#pragma mark - FeedWidget

void FeedWidget::quickLookImage(const QString &imagePath)
{
    static QuickLookHandler *qlh;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        qlh = [QuickLookHandler new];
    });

    qlh.imageFileUrl = [NSURL fileURLWithPath:imagePath.toNSString()];
    [[(__bridge NSView *)reinterpret_cast<void *>(winId()) window] makeFirstResponder:qlh];

    auto qlPanel = QLPreviewPanel.sharedPreviewPanel;
    if (qlPanel.isVisible)
        [qlPanel reloadData];
    else
        [qlPanel makeKeyAndOrderFront:qlh];
}

void FeedWidget::showLocation(double latitude, double longitude)
{
    static auto const pinTitleEncoded = @"%F0%9F%92%A9";
    auto params = [NSString stringWithFormat:@"t=m&z=13&q=%@&ll=%lf,%lf", pinTitleEncoded, latitude, longitude];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[@"http://maps.apple.com/?" stringByAppendingString:params]]];
}

#include "feedwidget.h"

#import <AppKit/AppKit.h>
#import <Quartz/Quartz.h>
#include <QApplication>


// functions copied from qtbase/src/plugins/platforms/cocoa/qcocoahelpers.mm

int qt_mac_mainScreenHeight()
{
    // simplified original version
    return [NSScreen screens].firstObject.frame.size.height;
}

int qt_mac_flipYCoordinate(int y)
{
    return qt_mac_mainScreenHeight() - y;
}

NSRect qt_mac_flipRect(const QRect &rect)
{
    int flippedY = qt_mac_flipYCoordinate(rect.y() + rect.height());
    return NSMakeRect(rect.x(), flippedY, rect.width(), rect.height());
}


@interface QuickLookHandler : NSResponder <QLPreviewPanelDataSource, QLPreviewPanelDelegate>
@property (nonatomic, copy) NSURL *imageFileUrl;
@property (nonatomic) NSRect sourceRect;
@end

@implementation QuickLookHandler

- (BOOL)acceptsFirstResponder { return YES; }

#pragma mark - QLPreviewPanelController

- (BOOL)acceptsPreviewPanelControl:(QLPreviewPanel *__unused)panel { return YES; }
- (void)beginPreviewPanelControl:(QLPreviewPanel *)panel { panel.dataSource = self; panel.delegate = self; }
- (void)endPreviewPanelControl:(QLPreviewPanel *)panel
{
    panel.dataSource = nil;
    panel.delegate = nil;

    for (auto w : qApp->topLevelWidgets())
    {
        if (w->objectName() == QLatin1String("MainWindow"))
        {
            w->activateWindow();
            break;
        }
    }
}

#pragma mark - QLPreviewPanelDataSource

- (NSInteger)numberOfPreviewItemsInPreviewPanel:(QLPreviewPanel *__unused)panel { return 1; }
- (id<QLPreviewItem>)previewPanel:(QLPreviewPanel *__unused)panel previewItemAtIndex:(NSInteger __unused)index { return self.imageFileUrl; }

#pragma mark - QLPreviewPanelDelegate

- (NSRect)previewPanel:(QLPreviewPanel *__unused)panel sourceFrameOnScreenForPreviewItem:(id<QLPreviewItem> __unused)item { return self.sourceRect; }

@end


#pragma mark - FeedWidget

void FeedWidget::quickLookImage(const QString &imagePath, const QRect &rect)
{
    static QuickLookHandler *qlh;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        qlh = [QuickLookHandler new];
    });

    qlh.imageFileUrl = [NSURL fileURLWithPath:imagePath.toNSString()];
    qlh.sourceRect = qt_mac_flipRect(rect);
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

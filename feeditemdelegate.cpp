#include "feeditemdelegate.h"
#include "feeditemwidget.h"
#include "ui_feeditemwidget.h"
#include "feedlistmodel.h"
#include "feedimagecache.h"

#include <QPainter>
#include <QLabel>
#include <QResizeEvent>

class ImageWithLabelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWithLabelWidget(QWidget *parent = 0) : QWidget(parent), background(new QLabel(this)), backgroundCover(new QWidget(this)), label(new QLabel(this)), originlImage(0)
    {
        backgroundCover->setStyleSheet("QWidget { background-color: rgba(0, 0, 0, 0.33); }");

        label->setStyleSheet("QLabel { color: white; }");
        label->setAlignment(Qt::AlignCenter);
        label->setInputMethodHints(Qt::ImhMultiLine);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
        label->setOpenExternalLinks(true);
        label->setWordWrap(true);
    }

    void setResizedBackgroundImage()
    {
        if (originlImage)
            background->setPixmap(QPixmap::fromImage(originlImage->scaled(background->size(), Qt::KeepAspectRatioByExpanding)));
    }

    QLabel *background;
    QWidget *backgroundCover;
    QLabel *label;
    QImage *originlImage;

protected:
    void resizeEvent(QResizeEvent *e)
    {
        auto newSize = e->size();
        backgroundCover->resize(newSize);

        background->resize(newSize);
        setResizedBackgroundImage();

        label->adjustSize();
        auto centeredLabelOriginSize = (newSize - label->size()) / 2;
        label->move(centeredLabelOriginSize.width(), centeredLabelOriginSize.height());
    }
};

#include "feeditemdelegate.moc"


void FeedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QPaintDevice *paintDeviceOriginal = painter->device();

    auto r = option.rect;
    FeedItemWidget w;
    w.setGeometry(r);

    auto item = itemAtIndex(index);
    w.ui->dateTimeEdit->setDateTime(QDateTime::fromTime_t(item->timestamp));
    w.ui->reputationLcd->display(item->reputation);
    w.ui->commentsLcd->display(static_cast<int>(item->comments));

    painter->end();
    w.render(painter->device(), r.topLeft(), QRegion(0, 0, r.width(), r.height()), QWidget::RenderFlag::DrawChildren);
    painter->begin(paintDeviceOriginal);
}

QWidget *FeedItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
    auto w = new ImageWithLabelWidget(parent);
    w->label->installEventFilter(parent->parent()); // listview
    return w;
}

void FeedItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect.adjusted(0, 0, 0, -50));
}

void FeedItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto w = qobject_cast<ImageWithLabelWidget *>(editor);
    auto item = itemAtIndex(index);
    w->label->setText(item->message);

    FeedImageCache::getImageFromUrl(item->background, [w](QImage *image){
        w->originlImage = image;
        w->setResizedBackgroundImage();
    });
}

// private

FeedItem *FeedItemDelegate::itemAtIndex(const QModelIndex &index) const
{
    return static_cast<const FeedListModel *>(index.model())->itemAtModelIndex(index);
}

#include "feeditemdelegate.h"
#include "feeditemwidget.h"
#include "ui_feeditemwidget.h"
#include "feedlistmodel.h"
#include "feedimagecache.h"

#include <QPainter>
#include <QLabel>
#include <QResizeEvent>

#include <QtConcurrent/QtConcurrent>

class ImageWithLabelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWithLabelWidget(QWidget *parent = 0) : QWidget(parent), background(new QLabel(this)), backgroundCover(new QWidget(this)), label(new QLabel(this)), originalImage(0)
    {
        backgroundCover->setStyleSheet("QWidget { background-color: rgba(0, 0, 0, 0.5); }");

        label->setStyleSheet("QLabel { color: white; }");
        label->setAlignment(Qt::AlignCenter);
        label->setInputMethodHints(Qt::ImhMultiLine);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
        label->setOpenExternalLinks(true);
        label->setWordWrap(true);

        connect(&imageResizeWatcher, &QFutureWatcher<QPixmap>::finished, [this]{
            background->setPixmap(imageResizeWatcher.result());
        });
    }

    void setResizedBackgroundImage()
    {
        if (originalImage)
            imageResizeWatcher.setFuture(QtConcurrent::run(this, &ImageWithLabelWidget::resizedBackgroundImagePixmap));
    }

    QLabel *background;
    QWidget *backgroundCover;
    QLabel *label;
    QImage *originalImage;
    QFutureWatcher<QPixmap> imageResizeWatcher;

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

private:
    QPixmap resizedBackgroundImagePixmap() const { return QPixmap::fromImage(originalImage->scaled(background->size(), Qt::KeepAspectRatioByExpanding)); }
};

#include "feeditemdelegate.moc"


QSize FeedItemDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    return QSize(0, itemAtIndex(index) ? 300 : 50);
}

void FeedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto item = itemAtIndex(index);
    if (!item)
        return;

    QPaintDevice *paintDeviceOriginal = painter->device();

    auto r = option.rect.adjusted(0, 0, 25, 60);
    FeedItemWidget w;
    w.setGeometry(r);

    w.ui->dateTimeEdit->setDateTime(QDateTime::fromTime_t(item->timestamp));
    w.ui->reputationLcd->display(item->reputation);
    w.ui->commentsLcd->display(static_cast<int>(item->comments));

    painter->end();
    w.render(painter->device(), r.topLeft(), QRegion(0, 0, r.width(), r.height()), QWidget::RenderFlag::DrawChildren);
    painter->begin(paintDeviceOriginal);
}

QWidget *FeedItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &index) const
{
    if (!itemAtIndex(index))
        return 0;

    auto w = new ImageWithLabelWidget(parent);
    w->label->installEventFilter(parent->parent()); // listview
    return w;
}

void FeedItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
#ifdef Q_OS_WIN32
    int heightAdjust = -50;
#else
    int heightAdjust = -30;
#endif
    auto r = option.rect;
    if (itemAtIndex(index))
        r.adjust(0, 0, 0, heightAdjust);
    editor->setGeometry(r);
}

void FeedItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto item = itemAtIndex(index);

    static const QRegularExpression whitespaceRex(QLatin1String("\\s"));
    auto text = item->message, ahref = QString("<a_href"), linkFormat = QString(ahref + "=\"%1\">%1</a>");
    bool replaceHtmlWhitespace = false;
    int httpIndex = 0;
    forever
    {
        if ((httpIndex = text.indexOf(QLatin1String("http"), httpIndex, Qt::CaseInsensitive)) == -1)
            break;

        int linkEnd = text.indexOf(whitespaceRex, httpIndex);
        if (linkEnd == -1)
            linkEnd = text.length();

        int length = linkEnd - httpIndex;
        auto link = linkFormat.arg(text.mid(httpIndex, length));
        text.replace(httpIndex, length, link);
        httpIndex += link.length();

        replaceHtmlWhitespace = true;
    }
    if (replaceHtmlWhitespace)
        text.replace(QChar::LineFeed, QLatin1String("<br>")).replace(QChar::Space, QLatin1String("&nbsp;")).replace(ahref, QLatin1String("<a href"));

    auto w = qobject_cast<ImageWithLabelWidget *>(editor);
    w->label->setText(text);

    FeedImageCache::getImageForItem(item, [w](QImage *image){
        w->originalImage = image;
        w->setResizedBackgroundImage();
    });
}

// private

FeedItem *FeedItemDelegate::itemAtIndex(const QModelIndex &index) const
{
    return static_cast<const FeedListModel *>(index.model())->itemAtModelIndex(index);
}

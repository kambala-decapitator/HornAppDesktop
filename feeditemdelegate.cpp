#include "feeditemdelegate.h"
#include "feeditemwidget.h"
#include "ui_feeditemwidget.h"
#include "feedlistmodel.h"

#include <QPainter>
#include <QLabel>
#include <QResizeEvent>

#include <QNetworkAccessManager>
#include <QNetworkReply>

class ImageWithLabelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWithLabelWidget(QWidget *parent = 0) : QWidget(parent), background(new QLabel(this)), backgroundCover(new QWidget(this)), label(new QLabel(this))
    {
        backgroundCover->setStyleSheet("QWidget { background-color: rgba(0, 0, 0, 0.33); }");

        label->setStyleSheet("QLabel { color: white; }");
        label->setAlignment(Qt::AlignCenter);
        label->setInputMethodHints(Qt::ImhMultiLine);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
        label->setOpenExternalLinks(true);
        label->setWordWrap(true);
    }

    void setResizedBackgroundImage(const QPixmap &pixmap) { background->setPixmap(pixmap.scaled(background->size(), Qt::KeepAspectRatioByExpanding)); }

    QLabel *background;
    QWidget *backgroundCover;
    QLabel *label;

protected:
    void resizeEvent(QResizeEvent *e)
    {
        auto newSize = e->size();
        backgroundCover->resize(newSize);

        background->resize(newSize);
        auto pixmap = background->pixmap();
        if (pixmap)
            setResizedBackgroundImage(*pixmap);

        label->adjustSize();
        auto centeredLabelOriginSize = (newSize - label->size()) / 2;
        label->move(centeredLabelOriginSize.width(), centeredLabelOriginSize.height());
    }
};

#include "feeditemdelegate.moc"


QNetworkAccessManager *FeedItemDelegate::_qnam;

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

    if (item->background.startsWith("http"))
    {
        if (!_qnam)
            _qnam = new QNetworkAccessManager;

        auto reply = _qnam->get(QNetworkRequest(QUrl(item->background)));
        connect(reply, &QNetworkReply::finished, [reply, w]{
            if (reply->error() == QNetworkReply::NoError)
            {
                auto imageData = reply->readAll();
                w->setResizedBackgroundImage(QPixmap::fromImage(QImage::fromData(imageData)));
            }
        });
    }
}

// private

FeedItem *FeedItemDelegate::itemAtIndex(const QModelIndex &index) const
{
    return static_cast<const FeedListModel *>(index.model())->itemAtModelIndex(index);
}

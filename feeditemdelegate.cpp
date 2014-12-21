#include "feeditemdelegate.h"
#include "feeditemwidget.h"
#include "ui_feeditemwidget.h"
#include "feedlistmodel.h"

#include <QPainter>
#include <QLabel>
#include <QResizeEvent>

class ImageWithLabelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWithLabelWidget(QWidget *parent = 0) : QWidget(parent), background(new QLabel(this)), label(new QLabel(this))
    {
        label->setAlignment(Qt::AlignCenter);
        label->setInputMethodHints(Qt::ImhMultiLine);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
        label->setOpenExternalLinks(true);
        label->setWordWrap(true);
    }

    QLabel *background, *label;

protected:
    void resizeEvent(QResizeEvent *e)
    {
        background->resize(e->size());

        label->adjustSize();
        QSize centeredLabelOriginSize = (e->size() - label->size()) / 2;
        label->move(centeredLabelOriginSize.width(), centeredLabelOriginSize.height());
    }
};

#include "feeditemdelegate.moc"


void FeedItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QPaintDevice *paintDeviceOriginal = painter->device();

    QRect r = option.rect;
    FeedItemWidget w;
    w.setGeometry(r);

    FeedItem *item = itemAtIndex(index);
    w.ui->dateTimeEdit->setDateTime(QDateTime::fromTime_t(item->timestamp));
    w.ui->reputationLcd->display(item->reputation);
    w.ui->commentsLcd->display(static_cast<int>(item->comments));

    painter->end();
    w.render(painter->device(), r.topLeft(), QRegion(0, 0, r.width(), r.height()), QWidget::RenderFlag::DrawChildren);
    painter->begin(paintDeviceOriginal);
}

QWidget *FeedItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
    return new ImageWithLabelWidget(parent);
}

void FeedItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect.adjusted(0, 0, 0, -50));
}

void FeedItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    qobject_cast<ImageWithLabelWidget *>(editor)->label->setText(itemAtIndex(index)->message);
}

// private

FeedItem *FeedItemDelegate::itemAtIndex(const QModelIndex &index) const
{
    return static_cast<const FeedListModel *>(index.model())->itemAtModelIndex(index);
}

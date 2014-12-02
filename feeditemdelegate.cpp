#include "feeditemdelegate.h"
#include "feeditemwidget.h"
#include "ui_feeditemwidget.h"
#include "feedlistmodel.h"

#include <QPainter>
#include <QLabel>

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
    QLabel *l = new QLabel(parent);
    l->setAlignment(Qt::AlignCenter);
    l->setInputMethodHints(Qt::ImhMultiLine);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    l->setOpenExternalLinks(true);
    l->setWordWrap(true);
    return l;
}

void FeedItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    editor->setGeometry(option.rect.adjusted(0, 0, 0, -50));
    if (!qobject_cast<QLabel *>(editor)->text().isEmpty())
        centerWidget(editor);
}

void FeedItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    qobject_cast<QLabel *>(editor)->setText(itemAtIndex(index)->message);
    centerWidget(editor);
}

// private

FeedItem *FeedItemDelegate::itemAtIndex(const QModelIndex &index) const
{
    return static_cast<const FeedListModel *>(index.model())->itemAtModelIndex(index);
}

void FeedItemDelegate::centerWidget(QWidget *w) const
{
    QRect r = w->geometry();
    w->adjustSize();
    QSize size = w->size();
    w->move(r.center().x() - size.width() / 2, r.center().y() - size.height() / 2);
}

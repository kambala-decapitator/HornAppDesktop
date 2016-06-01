#include "feedlistmodel.h"

#include <QSize>

#include <QGeoPositionInfoSource>

QVariant FeedListModel::data(const QModelIndex &index, int role) const
{
    auto item = itemAtModelIndex(index);
    switch (role)
    {
    case Qt::DisplayRole:
        return item->message;
    case Qt::SizeHintRole:
        return QSize(0, 250);
    case Qt::ToolTipRole:
    {
        QString s;
        if (_geoSource)
        {
            s = item->coordinates.isValid() ? tr("%1 km").arg(qRound(_geoSource->lastKnownPosition().coordinate().distanceTo(item->coordinates) / 10000) * 10)
                                            : tr("somewhere");
            s += "\n\n";
        }
        return s + item->tags.join(QChar(QChar::LineFeed));
    }
    default:
        return QVariant();
    }
}

void FeedListModel::setDataSource(const TextItemList &feed)
{
    beginResetModel();

    qDeleteAll(_dataSource);
    _dataSource = feed;

    endResetModel();
}

void FeedListModel::appendItems(const TextItemList &feed)
{
    beginInsertRows(QModelIndex(), _dataSource.size(), _dataSource.size() + feed.size() - 1);
    _dataSource.append(feed);
    endInsertRows();
}

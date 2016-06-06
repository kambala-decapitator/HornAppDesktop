#include "feedlistmodel.h"
#include "requestmanager.h"

#include <QSize>

#include <QGeoPositionInfoSource>

QVariant FeedListModel::data(const QModelIndex &index, int role) const
{
    auto item = itemAtModelIndex(index);
    if (!item)
        return QVariant();

    switch (role)
    {
    case Qt::ToolTipRole:
    {
        QGeoCoordinate coordinate;
        if (_geoSource)
            coordinate = _geoSource->lastKnownPosition().coordinate();
        else if (RequestManager::hasIpGeo())
            coordinate = QGeoCoordinate(RequestManager::ipLatitude, RequestManager::ipLongitude);

        QString distance;
        if (coordinate.isValid())
        {
            distance = item->coordinates.isValid() ? tr("%1 km").arg(qRound(coordinate.distanceTo(item->coordinates) / 10000) * 10)
                                                   : tr("somewhere");
            distance += "\n\n";
        }
        return distance + item->tags.join(QChar(QChar::LineFeed));
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

FeedItem *FeedListModel::itemAtModelIndex(const QModelIndex &index) const
{
    return index.row() < _dataSource.size() ? static_cast<FeedItem *>(_dataSource.at(index.row())) : 0;
}

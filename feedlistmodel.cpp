#include "feedlistmodel.h"

#include <QSize>

QVariant FeedListModel::data(const QModelIndex &index, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        return _dataSource.at(index.row())->message;
    case Qt::SizeHintRole:
        return QSize(0, 250);
    default:
        return QVariant();
    }
}

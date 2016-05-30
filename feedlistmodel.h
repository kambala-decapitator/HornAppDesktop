#ifndef FEEDLISTMODEL_H
#define FEEDLISTMODEL_H

#include <QAbstractListModel>

#include "feeditem.h"

class QGeoPositionInfoSource;

class FeedListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FeedListModel(QGeoPositionInfoSource *geoSource, QObject *parent = 0) : QAbstractListModel(parent), _geoSource(geoSource) {}
    virtual ~FeedListModel() { qDeleteAll(_dataSource); }

    int rowCount(const QModelIndex &/*parent*/ = QModelIndex()) const { return _dataSource.size(); }
    QVariant data(const QModelIndex &index, int role) const;

    void setDataSource(const TextItemList &feed);
    void appendItems(const TextItemList &feed);

    FeedItem *itemAtModelIndex(const QModelIndex &index) const { return static_cast<FeedItem *>(_dataSource.at(index.row())); }

private:
    TextItemList _dataSource;
    QGeoPositionInfoSource *_geoSource;
};

#endif // FEEDLISTMODEL_H

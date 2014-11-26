#ifndef FEEDLISTMODEL_H
#define FEEDLISTMODEL_H

#include <QAbstractListModel>

#include "feeditem.h"

class FeedListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FeedListModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const { Q_UNUSED(parent); return _dataSource.size(); }
    QVariant data(const QModelIndex &index, int role) const;

    void setDataSource(const QList<FeedItem> &feed) { beginResetModel(); _dataSource = feed; endResetModel(); }

signals:

public slots:

private:
    QList<FeedItem> _dataSource;
};

#endif // FEEDLISTMODEL_H

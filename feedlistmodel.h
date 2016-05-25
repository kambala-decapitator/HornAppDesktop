#ifndef FEEDLISTMODEL_H
#define FEEDLISTMODEL_H

#include <QAbstractListModel>

#include "feeditem.h"

class FeedListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FeedListModel(QObject *parent = 0) : QAbstractListModel(parent) {}
    virtual ~FeedListModel() { qDeleteAll(_dataSource); }

    int rowCount(const QModelIndex &/*parent*/ = QModelIndex()) const { return _dataSource.size(); }
    QVariant data(const QModelIndex &index, int role) const;

    void setDataSource(const TextItemList &feed);
    void appendItems(const TextItemList &feed);

    FeedItem *itemAtModelIndex(const QModelIndex &index) const { return static_cast<FeedItem *>(_dataSource.at(index.row())); }

private:
    TextItemList _dataSource;
};

#endif // FEEDLISTMODEL_H

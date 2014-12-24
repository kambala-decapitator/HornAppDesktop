#include "ui_widget.h"
#include "widget.h"
#include "requestmanager.h"
#include "feedlistmodel.h"
#include "feeditemdelegate.h"
#include "commentswidget.h"
#include "feedimagecache.h"

#include <QLabel>
#include <QMenu>

#ifndef QT_NO_DEBUG
#include <QDebug>
#endif

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget), _feedModel(new FeedListModel(this))
{
    ui->setupUi(this);

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listView->setModel(_feedModel);
    ui->listView->setItemDelegate(new FeedItemDelegate(this));

    connect(ui->listView, &QListView::doubleClicked, [this](const QModelIndex &index) {
        FeedItem *item = _feedModel->itemAtModelIndex(index);
        RequestManager::instance().requestComments(item->id, [item, this](const TextItemList &comments) {
            CommentsWidget *w = new CommentsWidget(item, comments, this, Qt::Window);
            w->show();
        });
    });
    connect(ui->listView, &QListView::customContextMenuRequested, [this](const QPoint &p) {
        QModelIndex index = ui->listView->indexAt(p);
        if (index.isValid())
        {
            QAction *action = new QAction(tr("Open image"), ui->listView);
            connect(action, &QAction::triggered, [index, this]{
                FeedItem *item = _feedModel->itemAtModelIndex(index);
                FeedImageCache::getImageFromUrl(item->background, [this](QImage *image) {
                    QLabel *imageWindow = new QLabel(this, Qt::Window);
                    imageWindow->setAttribute(Qt::WA_DeleteOnClose);
                    imageWindow->setPixmap(QPixmap::fromImage(*image));
                    imageWindow->adjustSize();
                    imageWindow->setMinimumSize(imageWindow->size());
                    imageWindow->setWindowTitle(tr("Image"));
                    imageWindow->show();
                });
            });
            QMenu::exec(QList<QAction *>() << action, ui->listView->mapToGlobal(p));
        }
    });

    RequestManager::instance().requestNewPosts([this](const TextItemList &feed) {
        _feedModel->setDataSource(feed);
        for (int i = 0; i < feed.size(); ++i)
            ui->listView->openPersistentEditor(_feedModel->index(i));
    });
}

Widget::~Widget()
{
    delete ui;
}

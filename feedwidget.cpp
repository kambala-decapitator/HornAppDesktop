#include "ui_feedwidget.h"
#include "feedwidget.h"
#include "requestmanager.h"
#include "feedlistmodel.h"
#include "feeditemdelegate.h"
#include "commentswidget.h"
#include "feedimagecache.h"

#include <QLabel>
#include <QMenu>
#include <QProgressDialog>
#include <QInputDialog>
#include <QMessageBox>

#include <QTimer>

FeedWidget::FeedWidget(const QString &requestPart, QWidget *parent) : QWidget(parent), ui(new Ui::FeedWidget), _feedModel(new FeedListModel(this)), _requestPart(requestPart), _requestFeedOnFirstShow(true)
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
}

FeedWidget::~FeedWidget()
{
    delete ui;
}

void FeedWidget::requestFeed()
{
    QProgressDialog *progress = new QProgressDialog(tr("Updating feed..."), QString(), 0, 0, this, Qt::CustomizeWindowHint);
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    RequestManager::instance().requestPostsWithRequestPart(_requestPart, [progress, this](const TextItemList &feed) {
        _feedModel->setDataSource(feed);
        progress->deleteLater();

        for (int i = 0; i < feed.size(); ++i)
        {
            qApp->processEvents();
            ui->listView->openPersistentEditor(_feedModel->index(i));
        }
    });
}

void FeedWidget::showEvent(QShowEvent *)
{
    if (_requestFeedOnFirstShow)
    {
        _requestFeedOnFirstShow = false;
        requestFeed();
    }
}
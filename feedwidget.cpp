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
#include <QClipboard>

#include <QTimer>

#include <QGeoPositionInfoSource>

FeedWidget::FeedWidget(const QString &requestPart, QGeoPositionInfoSource *geoSource, QWidget *parent) : QWidget(parent), ui(new Ui::FeedWidget), _feedModel(new FeedListModel(this)), _requestPart(requestPart), _requestFeedOnFirstShow(true), _geoSource(geoSource)
{
    ui->setupUi(this);

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listView->setModel(_feedModel);
    ui->listView->setItemDelegate(new FeedItemDelegate(this));

    connect(ui->listView, &QListView::doubleClicked, [this](const QModelIndex &index) {
        auto item = _feedModel->itemAtModelIndex(index);
        RequestManager::instance().requestComments(item->id, [item, this](const TextItemList &comments) {
            auto w = new CommentsWidget(item, comments, false, QSet<quint32>(), this, Qt::Window);
            w->show();

            QString distanceStr = item->coordinates.isValid() ? tr("%1 km").arg(qRound(_geoSource->lastKnownPosition().coordinate().distanceTo(item->coordinates) / 10000) * 10)
                                                              : tr("no geolocation");
            w->setWindowTitle(w->windowTitle() + QString(" (%1)").arg(distanceStr));
        });
    });
    connect(ui->listView, &QListView::customContextMenuRequested, [this](const QPoint &p) {
        auto index = ui->listView->indexAt(p);
        if (index.isValid())
        {
            auto action = new QAction(tr("Open image"), ui->listView);
            connect(action, &QAction::triggered, [index, this]{
                auto imageUrl = _feedModel->itemAtModelIndex(index)->background;
                FeedImageCache::getImageFromUrl(imageUrl, [imageUrl, this](QImage *image) {
                    auto imageWindow = new QLabel(this, Qt::Dialog);
                    imageWindow->setAttribute(Qt::WA_DeleteOnClose);
                    imageWindow->setPixmap(QPixmap::fromImage(*image));
                    imageWindow->setScaledContents(true);
                    imageWindow->setContextMenuPolicy(Qt::ActionsContextMenu);
                    imageWindow->installEventFilter(this);
                    imageWindow->adjustSize();
                    imageWindow->resize(imageWindow->height() * image->width() / image->height(), imageWindow->height());
                    imageWindow->show();

                    auto copyImageUrlAction = new QAction(tr("Copy URL"), imageWindow);
                    copyImageUrlAction->setShortcut(QKeySequence::Copy);
                    connect(copyImageUrlAction, &QAction::triggered, [imageUrl]{
                        qApp->clipboard()->setText(imageUrl);
                    });
                    imageWindow->addAction(copyImageUrlAction);

                    auto copyImageAction = new QAction(tr("Copy Image"), imageWindow);
                    copyImageAction->setShortcut({"Ctrl+Shift+C"});
                    connect(copyImageAction, &QAction::triggered, [image]{
                        qApp->clipboard()->setImage(*image);
                    });
                    imageWindow->addAction(copyImageAction);
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
    auto progress = new QProgressDialog(tr("Updating feed..."), QString(), 0, 0, this, Qt::CustomizeWindowHint);
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
        ui->listView->scrollToTop();
    });
}

void FeedWidget::loadNextPosts()
{
    auto progress = new QProgressDialog(tr("Updating feed..."), QString(), 0, 0, this, Qt::CustomizeWindowHint);
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    RequestManager::instance().requestPostsWithRequestPart(_requestPart, [progress, this](const TextItemList &feed) {
        int oldSize = _feedModel->rowCount();
        _feedModel->appendItems(feed);
        progress->deleteLater();

        for (int i = oldSize; i < _feedModel->rowCount(); ++i)
        {
            qApp->processEvents();
            ui->listView->openPersistentEditor(_feedModel->index(i));
        }
    }, _feedModel->itemAtModelIndex(_feedModel->index(_feedModel->rowCount() - 1))->id);
}

bool FeedWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() != QEvent::KeyPress || static_cast<QKeyEvent *>(e)->key() != Qt::Key_Escape)
        return QWidget::eventFilter(o, e);

    qobject_cast<QWidget *>(o)->close();
    return true;
}

void FeedWidget::showEvent(QShowEvent *)
{
    if (_requestFeedOnFirstShow)
    {
        _requestFeedOnFirstShow = false;
        requestFeed();
    }
}

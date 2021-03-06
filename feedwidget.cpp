#include "ui_feedwidget.h"
#include "feedwidget.h"
#include "requestmanager.h"
#include "feedlistmodel.h"
#include "feeditemdelegate.h"
#include "feedimagecache.h"
#include "commentsdialog.h"

#include <QMenu>
#include <QProgressDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QPushButton>

#include <QTimer>

FeedWidget::FeedWidget(const QString &requestPart, QGeoPositionInfoSource *geoSource, QWidget *parent) : QWidget(parent), ui(new Ui::FeedWidget), _feedModel(new FeedListModel(geoSource, this)), _requestPart(requestPart)
{
    ui->setupUi(this);

    installEventFilter(this);
    ui->listView->viewport()->installEventFilter(this);

    ui->listView->setModel(_feedModel);
    ui->listView->setItemDelegate(new FeedItemDelegate(this));

    connect(ui->listView, &QListView::doubleClicked, [this](const QModelIndex &index) {
        auto item = _feedModel->itemAtModelIndex(index);
        if (item)
            CommentsDialog::instance().showComments(item->id);
    });
    connect(ui->listView, &QListView::customContextMenuRequested, [this](const QPoint &p) {
        auto index = ui->listView->indexAt(p);
        if (!index.isValid())
            return;

        auto item = _feedModel->itemAtModelIndex(index);
        if (!item)
            return;

        auto openImageAction = new QAction(tr("Открыть изображение"), ui->listView);
        connect(openImageAction, SIGNAL(triggered(bool)), SLOT(openImage()));
        _openImageModelIndex = index;

        QList<QAction *> actions = {openImageAction};
#ifdef Q_OS_MACOS
        if (item->coordinates.isValid())
        {
            auto showLocationAction = new QAction(tr("Показать местоположение"), ui->listView);
            connect(showLocationAction, &QAction::triggered, [item, this]{
                showLocation(item->coordinates.latitude(), item->coordinates.longitude());
            });
            actions << showLocationAction;
        }
#endif
        QMenu::exec(actions, ui->listView->mapToGlobal(p));
    });
}

FeedWidget::~FeedWidget()
{
    delete ui;
}

void FeedWidget::requestFeed()
{
    requestFeed(0, [this](const TextItemList &feed){
        _feedModel->setDataSource(feed);
        return 0;
    }, [this]{ ui->listView->scrollToTop(); });
}

bool FeedWidget::eventFilter(QObject *o, QEvent *e)
{
    if (_requestFeedOnFirstShow && o == this && e->type() == QEvent::Show)
    {
        _requestFeedOnFirstShow = false;
        QTimer::singleShot(0, this, SLOT(requestFeed()));
        return true;
    }

    if (e->type() == QEvent::KeyPress)
    {
        int key = static_cast<QKeyEvent *>(e)->key();
        if (o == ui->listView->viewport())
        {
            if (key == Qt::Key_Space)
            {
                _openImageModelIndex = ui->listView->indexAt(QPoint());
                openImage();
                return true;
            }
        }
#ifdef CUSTOM_IMAGE_WINDOW
        else if (o == _imageWindow)
        {
            if (key == Qt::Key_Escape)
            {
                _imageWindow->close();
                return true;
            }
        }
#endif
    }
    return QWidget::eventFilter(o, e);
}

void FeedWidget::loadNextPosts()
{
    requestFeed(_feedModel->itemAtModelIndex(_feedModel->index(_feedModel->rowCount() - 2))->id, [this](const TextItemList &feed){
        int oldSize = _feedModel->rowCount() - 1;
        _feedModel->appendItems(feed);
        return oldSize;
    });
}

void FeedWidget::openImage()
{
    if (!_openImageModelIndex.isValid())
        return;

    auto item = _feedModel->itemAtModelIndex(_openImageModelIndex);
    if (!item)
        return;

#ifdef CUSTOM_IMAGE_WINDOW
    auto imageUrl = item->background;
    FeedImageCache::getImageForItem(item, [imageUrl, this](QImage *image) {
        if (!_imageWindow)
        {
            _imageWindow = new QLabel(this, Qt::Dialog);
            _imageWindow->setAttribute(Qt::WA_DeleteOnClose);
            _imageWindow->setScaledContents(true);
            _imageWindow->setContextMenuPolicy(Qt::ActionsContextMenu);
            _imageWindow->installEventFilter(this);
        }
        _imageWindow->setPixmap(QPixmap::fromImage(*image));
        _imageWindow->adjustSize();
        _imageWindow->resize(_imageWindow->height() * image->width() / image->height(), _imageWindow->height());
        _imageWindow->show();

        for (auto action : _imageWindow->actions())
            _imageWindow->removeAction(action);

        auto copyImageUrlAction = new QAction(tr("Копировать ссылку"), _imageWindow);
        copyImageUrlAction->setShortcut(QKeySequence::Copy);
        connect(copyImageUrlAction, &QAction::triggered, [imageUrl]{
            qApp->clipboard()->setText(imageUrl);
        });
        _imageWindow->addAction(copyImageUrlAction);

        auto copyImageAction = new QAction(tr("Копировать изображение"), _imageWindow);
        copyImageAction->setShortcut({"Ctrl+Shift+C"});
        connect(copyImageAction, &QAction::triggered, [image]{
            qApp->clipboard()->setImage(*image);
        });
        _imageWindow->addAction(copyImageAction);
    });
#else
    auto r = ui->listView->visualRect(_openImageModelIndex);
    r.moveTopLeft(ui->listView->mapToGlobal(r.topLeft()));
    quickLookImage(FeedImageCache::savePathForItem(item), r);
#endif

    _openImageModelIndex = QModelIndex();
}

void FeedWidget::requestFeed(quint32 postIdForOlderFeed, std::function<int(const TextItemList &)> processFeedCallback, std::function<void(void)> afterDisplayingFeedCallback)
{
    auto progress = new QProgressDialog(tr("Обновление ленты…"), QString(), 0, 0, this, Qt::CustomizeWindowHint);
    progress->setWindowModality(Qt::WindowModal);
    progress->show();

    RequestManager::instance().requestPostsWithRequestPart(_requestPart, [processFeedCallback, afterDisplayingFeedCallback, progress, this](const TextItemList &feed) {
        progress->deleteLater();
        if (feed.isEmpty())
            return;
        int startIndex = processFeedCallback(feed);

        auto loadNextPostsButton = new QPushButton(tr("Загрузить следующие"), this);
        connect(loadNextPostsButton, SIGNAL(clicked()), SLOT(loadNextPosts()));
        ui->listView->setIndexWidget(_feedModel->index(_feedModel->rowCount() - 1), loadNextPostsButton);

        for (int i = startIndex; i < _feedModel->rowCount(); ++i)
        {
            qApp->processEvents();
            ui->listView->openPersistentEditor(_feedModel->index(i));
        }
        afterDisplayingFeedCallback();
    }, postIdForOlderFeed);
}

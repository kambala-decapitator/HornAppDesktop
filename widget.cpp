#include "ui_widget.h"
#include "widget.h"
#include "requestmanager.h"
#include "feedlistmodel.h"
#include "feeditemdelegate.h"
#include "commentswidget.h"
#include "feedimagecache.h"

#include <QLabel>
#include <QMenu>
#include <QProgressDialog>

#include <QTimer>

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget), _feedModel(new FeedListModel(this))
{
    ui->setupUi(this);

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listView->setModel(_feedModel);
    ui->listView->setItemDelegate(new FeedItemDelegate(this));

#ifdef Q_OS_MAC
    ui->refreshButton->setShortcut(QKeySequence("Ctrl+R"));
#else
    ui->refreshButton->setShortcut(QKeySequence::Refresh);
#endif

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

    QTimer *refreshTimer = new QTimer;
    refreshTimer->setInterval(200 * 1000);
    connect(refreshTimer, &QTimer::timeout, ui->refreshButton, &QPushButton::click);

    connect(ui->refreshButton, &QPushButton::clicked, [refreshTimer, this]{
        QProgressDialog *progress = new QProgressDialog(tr("Updating feed..."), QString(), 0, 0, 0, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        progress->setWindowModality(Qt::ApplicationModal);
        progress->show();

        RequestManager::instance().requestNewPosts([refreshTimer, progress, this](const TextItemList &feed) {
            _feedModel->setDataSource(feed);
            delete progress;

            for (int i = 0; i < feed.size(); ++i)
            {
                ui->listView->openPersistentEditor(_feedModel->index(i));
                qApp->processEvents();
            }

            refreshTimer->start();
        });
    });

    ui->refreshButton->click(); // refresh feed on start
}

Widget::~Widget()
{
    delete ui;
}

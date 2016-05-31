#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "feedwidget.h"
#include "requestmanager.h"
#include "notificationsdialog.h"

#include <QSettings>

#include <QTabWidget>
#include <QInputDialog>
#include <QMessageBox>

#include <QGeoPositionInfoSource>

static const QLatin1String MainWindowSettingsKey("MainWindow"), NotificationsDialogSettingsKey("NotificationsDialog");
static const QLatin1String WindowPositionSettingsKey("pos"), WindowSizeSettingsKey("size");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), _tabWidget(new QTabWidget(this)), _notificationsDlg(new NotificationsDialog(this))
{
    ui->setupUi(this);
    static_cast<QBoxLayout *>(ui->centralwidget->layout())->insertWidget(0, _tabWidget);
    installEventFilter(this);

    QSettings settings;
    for (auto windowData : windowsToRestoreGeomentry())
    {
        settings.beginGroup(windowData.first);

        auto pos = settings.value(WindowPositionSettingsKey);
        if (pos.isValid())
            windowData.second->move(pos.toPoint());

        auto size = settings.value(WindowSizeSettingsKey);
        if (size.isValid())
            windowData.second->resize(size.toSize());

        settings.endGroup();
    }

    _notificationsDlg->installEventFilter(this);
    _notificationsDlg->show();

#ifdef Q_OS_MACX
    ui->menubar->insertMenu(ui->menuHelp->menuAction(), new QMenu(tr("Edit"), ui->menubar));
#endif

    ui->actionNewPost->setShortcut(QKeySequence::New);
    ui->actionRefreshFeed->setShortcut(QKeySequence::Refresh);

    if ((_geoSource = QGeoPositionInfoSource::createDefaultSource(this)))
        _geoSource->startUpdates();

    auto tabs = QList<QPair<QString, QString>>({{tr("New"), "HornNew"}});
    for (const auto &tab : QList<QPair<QString, QString>>({{tr("Commented"), "/Commented"}, {tr("Liked"), "/Liked"}, {tr("My"), QString()}}))
        tabs << qMakePair(tab.first, "User/" + RequestManager::userHashIdentifier + tab.second + "/Horn");
    tabs << qMakePair(tr("Top"), QLatin1String("HornTop"));
    for (const auto &tab : tabs)
    {
        auto fw = new FeedWidget(tab.second, _geoSource, this);
        _tabWidget->addTab(fw, tab.first);
    }

    connect(ui->actionNewPost, &QAction::triggered,   this, &MainWindow::createNewPost);
    connect(ui->newPostButton, &QPushButton::clicked, this, &MainWindow::createNewPost);

    connect(ui->actionRefreshFeed, &QAction::triggered,   this, &MainWindow::refreshCurrentFeed);
    connect(ui->refreshFeedButton, &QPushButton::clicked, this, &MainWindow::refreshCurrentFeed);

    connect(ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

    connect(ui->actionNotifications, &QAction::triggered, [this](bool checked){
        _notificationsDlg->setVisible(checked);
    });

    connect(ui->loadNextPostsButton, &QPushButton::clicked, [this]{
        feedWidgetWithIndex(_tabWidget->currentIndex())->loadNextPosts();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
    if (o == _notificationsDlg && e->type() == QEvent::Hide)
    {
        ui->actionNotifications->setChecked(false);
        return true;
    }
    else if (o == this && e->type() == QEvent::Close)
    {
        QSettings settings;
        for (auto windowData : windowsToRestoreGeomentry())
        {
            settings.beginGroup(windowData.first);
            settings.setValue(WindowPositionSettingsKey, windowData.second->pos());
            settings.setValue(WindowSizeSettingsKey, windowData.second->size());
            settings.endGroup();
        }
    }
    return QMainWindow::eventFilter(o, e);
}

void MainWindow::createNewPost()
{
    auto message = QInputDialog::getText(this, tr("New Post"), tr("Enter your message:")).trimmed();
    if (!message.isEmpty())
        RequestManager::instance().createPost(message, QStringList({"Various"}), qQNaN(), qQNaN(), [this](bool ok){
            if (ok)
                refreshFeedWithIndex(0);
            else
                QMessageBox::critical(this, QString(), tr("Error creating new post"));
        });
}

void MainWindow::refreshCurrentFeed()
{
    refreshFeedWithIndex(_tabWidget->currentIndex());
}

void MainWindow::refreshFeedWithIndex(int index)
{
    feedWidgetWithIndex(index)->requestFeed();
}

FeedWidget *MainWindow::feedWidgetWithIndex(int index)
{
    return static_cast<FeedWidget *>(_tabWidget->widget(index));
}

QList<QPair<QLatin1String, QWidget *>> MainWindow::windowsToRestoreGeomentry()
{
    return {qMakePair(MainWindowSettingsKey, this), qMakePair(NotificationsDialogSettingsKey, _notificationsDlg)};
}

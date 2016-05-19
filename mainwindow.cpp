#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "feedwidget.h"
#include "requestmanager.h"
#include "notificationsdialog.h"

#include <QTabWidget>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), _tabWidget(new QTabWidget(this)), _notificationsDlg(new NotificationsDialog(this))
{
    ui->setupUi(this);
    static_cast<QBoxLayout *>(ui->centralwidget->layout())->insertWidget(0, _tabWidget);

    _notificationsDlg->installEventFilter(this);
    _notificationsDlg->show();

#ifdef Q_OS_MACX
    ui->menubar->insertMenu(ui->menuHelp->menuAction(), new QMenu(tr("Edit"), ui->menubar));
#endif

    ui->actionNewPost->setShortcut(QKeySequence::New);
    ui->actionRefreshFeed->setShortcut(QKeySequence::Refresh);

    auto tabs = QList<QPair<QString, QString>>({{tr("New"), "HornNew"}});
    for (const auto &tab : QList<QPair<QString, QString>>({{tr("Commented"), "/Commented"}, {tr("Liked"), "/Liked"}, {tr("My"), QString()}}))
        tabs << qMakePair(tab.first, "User/" + RequestManager::userID + tab.second + "/Horn");
    tabs << qMakePair(tr("Top"), QLatin1String("HornTop"));
    for (const auto &tab : tabs)
    {
        auto fw = new FeedWidget(tab.second, this);
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
    if (o == _notificationsDlg && e->type() == QEvent::Close)
    {
        ui->actionNotifications->setChecked(false);
        return true;
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
    static_cast<FeedWidget *>(_tabWidget->widget(index))->requestFeed();
}

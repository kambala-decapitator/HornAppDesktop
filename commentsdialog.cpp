#include "commentsdialog.h"
#include "ui_commentsdialog.h"
#include "requestmanager.h"
#include "commentswidget.h"

#include <QKeyEvent>

CommentsDialog::CommentsDialog() : QDialog(nullptr, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint), ui(new Ui::CommentsDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    installEventFilter(this);

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &CommentsDialog::closeTab);
    connect(ui->loadNewButton, &QPushButton::clicked, [this](bool){
        showComments(qobject_cast<CommentsWidget *>(ui->tabWidget->currentWidget())->postId());
    });
    connect(ui->loadOldButton, &QPushButton::clicked, [this](bool){
        qobject_cast<CommentsWidget *>(ui->tabWidget->currentWidget())->loadOldComments();
    });
}

CommentsDialog::~CommentsDialog()
{
    delete ui;
}

void CommentsDialog::showComments(decltype(TextItem::id) postId, FeedItem *existingItem, const QSet<quint32> &highlightedComments)
{
    show();
    activateWindow();
    raise();

    int index = -1;
    CommentsWidget *commentsWidget = nullptr;
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        auto w = qobject_cast<CommentsWidget *>(ui->tabWidget->widget(i));
        if (w->postId() == postId)
        {
            index = i;
            commentsWidget = w;
            break;
        }
    }

    bool exists = index != -1;
    if (!commentsWidget)
    {
        commentsWidget = new CommentsWidget(postId, existingItem);
        index = ui->tabWidget->addTab(commentsWidget, QString::number(postId));
    }

    ui->tabWidget->setCurrentIndex(index);
    commentsWidget->loadNewComments(exists, highlightedComments);
}

void CommentsDialog::reject()
{
    close();
    QDialog::reject();
}

bool CommentsDialog::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type())
    {
    case QEvent::Close:
        while (ui->tabWidget->count())
            delete ui->tabWidget->widget(0);
        return true;
    case QEvent::KeyPress:
    {
        auto ke = static_cast<QKeyEvent *>(e);
        if (ke->matches(QKeySequence::Close))
        {
            closeTab(ui->tabWidget->currentIndex());
            return true;
        }
        break;
    }
    default:
        break;
    }
    return QDialog::eventFilter(o, e);
}

void CommentsDialog::closeTab(int index)
{
    delete ui->tabWidget->widget(index);
    if (!ui->tabWidget->count())
        close();
}

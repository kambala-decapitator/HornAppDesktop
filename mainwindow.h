#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class QTabWidget;
class NotificationsDialog;
class FeedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    bool eventFilter(QObject *o, QEvent *e);

private slots:
    void createNewPost();
    void refreshCurrentFeed();

private:
    Ui::MainWindow *ui;
    QTabWidget *_tabWidget;
    NotificationsDialog *_notificationsDlg;

    void refreshFeedWithIndex(int index);
    FeedWidget *feedWidgetWithIndex(int index);
};

#endif // MAINWINDOW_H

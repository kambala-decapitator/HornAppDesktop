#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}
class QTabWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void createNewPost();
    void refreshCurrentFeed();

private:
    Ui::MainWindow *ui;
    QTabWidget *_tabWidget;
};

#endif // MAINWINDOW_H

#ifndef NEWPOSTDIALOG_H
#define NEWPOSTDIALOG_H

#include <QDialog>

namespace Ui {
class NewPostDialog;
}
class QDoubleSpinBox;
class QGeoPositionInfoSource;

class NewPostDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewPostDialog(QGeoPositionInfoSource *geoSource, QWidget *parent = 0);
    virtual ~NewPostDialog();

private:
    Ui::NewPostDialog *ui;
    QGeoPositionInfoSource *_geoSource;

    static QString _imagesFilter;

    double randomCoordinateForSpinbox(QDoubleSpinBox *spinbox);
};

#endif // NEWPOSTDIALOG_H

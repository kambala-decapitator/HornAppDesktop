#include "newpostdialog.h"
#include "ui_newpostdialog.h"
#include "requestmanager.h"
#include "feedimagecache.h"

#include <QFileDialog>
#include <QImageReader>
#include <QMessageBox>

#include <QStandardPaths>
#include <QSettings>

#include <QGeoPositionInfoSource>

#include <cmath>

static const QLatin1String SettingsGroupName("NewPostDialog"), CustomLatitudeSettingsKey("customLatitude"), CustomLongitudeSettingsKey("customLongitude");

static const int MaxPostLength = 200;
QString NewPostDialog::_imagesFilter;

NewPostDialog::NewPostDialog(QGeoPositionInfoSource *geoSource, QWidget *parent) : QDialog(parent), ui(new Ui::NewPostDialog), _geoSource(geoSource)
{
    ui->setupUi(this);
    setWindowModality(Qt::WindowModal);

    if (!_geoSource && !RequestManager::ipLatitude && !RequestManager::ipLatitude)
    {
        ui->hiddenGeoRadioButton->setChecked(true);
        ui->currentGeoRadioButton->setEnabled(false);
    }

    QSettings settings;
    settings.beginGroup(SettingsGroupName);
    {
        auto latitude = settings.value(CustomLatitudeSettingsKey);
        if (latitude.isValid())
            ui->latitudeDoubleSpinBox->setValue(latitude.toDouble());

        auto longitude = settings.value(CustomLongitudeSettingsKey);
        if (longitude.isValid())
            ui->longitudeDoubleSpinBox->setValue(longitude.toDouble());
    }
    settings.endGroup();

    for (const auto &category : RequestManager::instance().categories())
    {
        auto item = new QListWidgetItem(category, ui->listWidget);
        item->setCheckState(Qt::Unchecked);
    }

    if (_imagesFilter.isEmpty())
    {
        QStringList extensions;
        for (const auto &extension : QImageReader::supportedImageFormats())
            extensions << QLatin1String("*.") + extension;
        _imagesFilter = tr("Поддерживаемые изображения (%1)").arg(extensions.join(QChar::Space));
    }

    connect(ui->selectImageButton, &QPushButton::clicked, [this]{
        auto path = QFileDialog::getOpenFileName(this, tr("Выберите изображение"), QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first(), _imagesFilter);
        if (!path.isEmpty())
            ui->imagePathLineEdit->setText(path);
    });

    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, [this]{
        ui->messageGroupBox->setTitle(QString::number(ui->plainTextEdit->toPlainText().size()));
    });

    connect(ui->buttonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [this](int){
        ui->buttonGroup->setExclusive(true);
        ui->customGeoGroupBox->setChecked(false);
    });
    connect(ui->customGeoGroupBox, &QGroupBox::clicked, [this](bool checked){
        ui->buttonGroup->setExclusive(!checked);
        if (checked)
        {
            ui->buttonGroup->checkedButton()->setChecked(false);
            ui->latitudeDoubleSpinBox->setFocus();
        }
        else
            ui->currentGeoRadioButton->setChecked(true);
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]{
        auto message = ui->plainTextEdit->toPlainText(), imagePath = ui->imagePathLineEdit->text();
        if (message.size() > MaxPostLength)
        {
            QMessageBox::critical(this, QString(), tr("Пост не может быть длиннее %1 символов.\nПожалуйста, допишите всё остальное в комментариях.").arg(MaxPostLength));
            return;
        }
        if (message.isEmpty() && imagePath.isEmpty())
        {
            QMessageBox::critical(this, QString(), tr("Пустое сообщение разрешено только при загрузке изображения"));
            return;
        }

        QStringList selectedCategories;
        for (int i = 0; i < ui->listWidget->count(); ++i)
        {
            auto item = ui->listWidget->item(i);
            if (item->checkState() == Qt::Checked)
                selectedCategories << RequestManager::instance().categoryIdFromName(item->text());
        }
        if (selectedCategories.isEmpty())
        {
            QMessageBox::critical(this, QString(), tr("Вы должны выбрать хотя бы одну категорию"));
            return;
        }
        if (selectedCategories.size() > RequestManager::maxCategories)
        {
            QMessageBox::critical(this, QString(), tr("Вы не можете выбрать более %1 категорий").arg(RequestManager::maxCategories));
            return;
        }

        auto createPost = [message, selectedCategories, this](quint32 imageId = 0, std::function<void(void)> successCallback = []{}){
            double latitude, longitude;
            if (ui->currentGeoRadioButton->isChecked())
            {
                if (_geoSource)
                {
                    auto lastPosition = _geoSource->lastKnownPosition().coordinate();
                    latitude  = lastPosition.latitude();
                    longitude = lastPosition.longitude();
                }
                else
                {
                    latitude  = RequestManager::ipLatitude;
                    longitude = RequestManager::ipLongitude;
                }
            }
            else if (ui->hiddenGeoRadioButton->isChecked())
            {
                latitude = longitude = qQNaN();
            }
            else if (ui->randomGeoRadioButton->isChecked())
            {
                latitude  = randomCoordinateForSpinbox(ui->latitudeDoubleSpinBox);
                longitude = randomCoordinateForSpinbox(ui->longitudeDoubleSpinBox);
            }
            else
            {
                latitude  = ui->latitudeDoubleSpinBox->value();
                longitude = ui->longitudeDoubleSpinBox->value();

                QSettings settings;
                settings.beginGroup(SettingsGroupName);
                settings.setValue(CustomLatitudeSettingsKey,  latitude);
                settings.setValue(CustomLongitudeSettingsKey, longitude);
                settings.endGroup();
            }

            RequestManager::instance().createPost(message, selectedCategories, latitude, longitude, imageId, [successCallback, this](bool ok){
                if (ok)
                {
                    successCallback();
                    accept();
                }
                else
                    QMessageBox::critical(this, QString(), tr("Ошибка создания поста"));
            });
        };
        if (imagePath.isEmpty())
        {
            createPost(0, []{}); // stupid MSVC doesn't like default params in lambda
            return;
        }

        auto imageReader = new QImageReader(imagePath);
        if (!imageReader->canRead())
        {
            delete imageReader;
            QMessageBox::critical(this, QString(), tr("Ошибка чтения файла изображения"));
            return;
        }

        RequestManager::instance().uploadImage(imageReader->device(), [createPost, imagePath, imageReader, this](const QJsonObject &json){
            delete imageReader;

            if (json.isEmpty())
            {
                QMessageBox::critical(this, QString(), tr("Ошибка загрузки изображения"));
                return;
            }

            auto itemIdString = json["id"].toString();
            createPost(itemIdString.toUInt(), [imagePath, itemIdString]{
                FeedImageCache::copyFileToCache(imagePath, itemIdString);
            });
        });
    });
}

NewPostDialog::~NewPostDialog()
{
    delete ui;
}

double NewPostDialog::randomCoordinateForSpinbox(QDoubleSpinBox *spinbox)
{
    double min = spinbox->minimum(), max = spinbox->maximum(), diff = min - max, foo;
    int r = qrand();
    return min + r % static_cast<int>(diff) + modf(r / diff, &foo);
}

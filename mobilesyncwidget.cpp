#include "mobilesyncwidget.h"
#include "mainwindow.h"
#include "requestmanager.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QApplication>

#include <QSettings>
#include <QRegularExpression>

#include <QNetworkInterface>
#include <QTcpServer>
#include <QTcpSocket>

#define USER_ID_KEY QStringLiteral("uid")
#define TOKEN_KEY   QStringLiteral("token")

void MobileSyncWidget::syncOrStartApp()
{
    QSettings settings;
    auto userId = settings.value(USER_ID_KEY), token = settings.value(TOKEN_KEY);
    if (userId.isValid() && token.isValid())
        startApp(userId.toString(), token.toString());
    else
    {
        auto w = new MobileSyncWidget;
        w->show();
    }
}

void MobileSyncWidget::startApp(const QString &userId, const QString &token)
{
    RequestManager::userHashIdentifier = userId;
    RequestManager::token = token;

    auto w = new MainWindow;
    w->show();
}

MobileSyncWidget::MobileSyncWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Синхронизация учётной записи"));
    setLayout(new QVBoxLayout);

    auto label = new QLabel{this};
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout()->addWidget(label);

    auto progress = new QProgressBar{this};
    progress->setRange(0, 0);
    layout()->addWidget(progress);

    // get list of computer's IPs
    QList<QHostAddress> lanIPs;
    for (const auto &interface : QNetworkInterface::allInterfaces())
    {
        if (!interface.isValid())
            continue;

        auto flags = interface.flags();
        if (!(flags & QNetworkInterface::IsRunning) || !(flags & QNetworkInterface::IsUp) || flags & QNetworkInterface::IsLoopBack || flags & QNetworkInterface::IsPointToPoint)
            continue;

        for (const auto &addressEntry : interface.addressEntries())
        {
            auto ip = addressEntry.ip();
            if (!ip.isNull() && !ip.isLoopback() && !ip.isMulticast())
                lanIPs << ip;
        }
    }
    std::sort(lanIPs.begin(), lanIPs.end(), [](QHostAddress a1, QHostAddress a2){
        return a1.protocol() == QAbstractSocket::IPv4Protocol || a2.protocol() != QAbstractSocket::IPv4Protocol;
    });

    QStringList ipStrings;
    for (const auto &ip : lanIPs)
        ipStrings << QStringLiteral("- ") + ip.toString();

    // open socket for listening
    auto server = new QTcpServer{this};
    connect(server, &QTcpServer::newConnection, [server, this]{
        auto socket = server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, [socket, server, this]{
            QLatin1String hexStringCapturedPattern{"([a-f\\d]+)"};
            QRegularExpression re{QString{"hornapp.+User/%1.+token=%1"}.arg(hexStringCapturedPattern)};
            auto match = re.match(QString::fromLatin1(socket->readAll()));
            if (match.hasMatch())
            {
                server->close();
                qApp->processEvents();

                auto userId = match.captured(1), token = match.captured(2);

                QSettings settings;
                settings.setValue(USER_ID_KEY, userId);
                settings.setValue(TOKEN_KEY,   token);

                QMessageBox::information(this, QString(), tr("Учётная запись синхронизирована!\nТеперь отключите HTTP прокси на вашем мобильном устройстве."));
                startApp(userId, token);
                close();
            }
        });
    });

    if (!server->listen(QHostAddress::Any) && !server->listen(QHostAddress::Any))
    {
        QMessageBox::critical(this, QString(), tr("Невозможно открыть сокет:\n%1\nПерезапустите приложение и попробуйте снова.").arg(server->errorString()));
        close();
        return;
    }

    label->setText(tr("1. Откройте HornApp на вашем мобильном устройстве и перейдите в Мои, Обсуждамемые или Понравившиеся.\n"
                      "2. Настройте HTTP прокси на вашем мобильном устройстве на порт %1 и один из следующих IP адресов:\n%2\n"
                      "3. Снова откройте HornApp и обновите выбранную вкладку.").arg(server->serverPort()).arg(ipStrings.join(QChar::LineFeed)));
}

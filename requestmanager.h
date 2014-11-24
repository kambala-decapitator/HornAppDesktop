#ifndef REQUESTMANAGER_H
#define REQUESTMANAGER_H

#include <QObject>
#include <QBuffer>

#include <QNetworkRequest>

class QNetworkAccessManager;

class RequestManager : public QObject
{
    Q_OBJECT
public:
    static RequestManager &instance()
    {
        static RequestManager obj;
        return obj;
    }

signals:

public slots:

private:
    explicit RequestManager(QObject *parent = 0);
    RequestManager(RequestManager &) {}

    QNetworkRequest requestFromUrlPart(const QString &urlPart, const QString &urlJsonText = QString());

private:
    QNetworkAccessManager *_qnam;
    QBuffer _patchRequestDataBuffer;
};

#endif // REQUESTMANAGER_H

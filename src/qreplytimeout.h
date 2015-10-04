#ifndef QREPLYTIMEOUT_H
#define QREPLYTIMEOUT_H

#include <QObject>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

class QReplyTimeout : public QObject
{
    Q_OBJECT
public:
//    explicit QReplyTimeout(QObject *parent = 0);
    QReplyTimeout(QNetworkReply* reply, const int timeout) : QObject(reply) {
        Q_ASSERT(reply);
        if (reply) {
            qDebug() << "singleShot()";
            QTimer::singleShot(timeout, this, SLOT(timeout()));
        }
    }
    ~QReplyTimeout(void) {
        qDebug() << "QReplyTimeout destructor";
    }

signals:

public slots:

private slots:
    void timeout() {
        QNetworkReply* reply = static_cast<QNetworkReply*>(parent());
        if (reply->isRunning()) {
            qDebug() << "reply->close()";
            reply->close();
        }
    }
};

#endif // QREPLYTIMEOUT_H

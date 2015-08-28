#include <QDebug>
#include <QString>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include "dogtracker.h"
#include "settings.h"

DogTracker::DogTracker(QObject *parent) :
    QObject(parent)
{
    qDebug()<<"DogTracker constructor";

    // start the timer for fetching dog position
    m_dogTrackingTimer.setInterval(10000);
    connect(&m_dogTrackingTimer, SIGNAL(timeout()), this, SLOT(fetchDogPosition()));
    m_dogTrackingTimer.start();
}

DogTracker::~DogTracker() {
    qDebug()<<"DogTracker destructor";
}

int DogTracker::dogTrackingInterval() const {
    return m_dogTrackingInterval;
}

void DogTracker::setdogTrackingInterval(int dogTrackingInterval) {
    m_dogTrackingInterval = dogTrackingInterval;
    qDebug()<<"Setting dog tracking interval to"<<dogTrackingInterval<<"msec";
    if(m_dogTrackingInterval > 0) {
        m_dogTrackingTimer.setInterval(m_dogTrackingInterval);
    } else {
        m_dogTrackingTimer.setInterval(10000);
    }
    emit dogTrackingIntervalChanged();
}

QGeoCoordinate DogTracker::currentDogPosition() const {
    return m_currentDogPosition;
}

void DogTracker::fetchDogPosition() {
    if(m_dogTrackingInterval > 0) {
        qDebug()<<"Fetching dog position";
        requestDogPosition();
    } else {
        qDebug()<<"Not fetching dog position";
    }
}

void DogTracker::requestDogPosition() {
    Settings settings;
    QEventLoop eventLoop;
    QNetworkAccessManager mgr;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    QNetworkRequest req(QUrl(settings.dogTrackingUrl() + "?password=" + settings.dogTrackingPassword()));
    QNetworkReply *reply = mgr.get(req);
    eventLoop.exec(); // blocks stack until "finished()" has been called

    if(reply->error() == QNetworkReply::NoError) {
        QString strReply = (QString)reply->readAll();
        qDebug() << "Response:" << strReply;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
        QJsonObject jsonObj = jsonResponse.object();
        if(jsonObj["version"].toString() == "0.1") {
            m_currentDogPosition.setLatitude(jsonObj["lat"].toString().toDouble());
            m_currentDogPosition.setLongitude(jsonObj["lon"].toString().toDouble());
            qDebug() << "Dog found at:" << m_currentDogPosition.latitude() << m_currentDogPosition.longitude();
            emit currentDogPositionChanged();
        } else {
            qDebug() << "Unsupported version:" << jsonObj["version"].toString();
        }
        delete reply;
    }
    else {
        qDebug() << "Cannot connect to server!" <<reply->errorString();
        delete reply;
    }
}

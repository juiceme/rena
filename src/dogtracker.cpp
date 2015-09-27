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

int DogTracker::dogPositionAccuracy() const {
    int scaledAccuracy = (int) m_dogPositionAccuracy;
    // Accuracy is between 1...50 (smaller is better) value 0 means invalid.
    // This scales the value to suit presentation as radius of circle.
    if(scaledAccuracy < 1) { return 50; }
    if(scaledAccuracy > 50) { scaledAccuracy = 50; }
    qDebug() << "Dog position accuracy:" << scaledAccuracy;
    return scaledAccuracy;
}

int DogTracker::dogPositionAge() const {
    double scaledAge = (double) m_dogPositionAge;
    // Age should be between 0...3600 seconds. However we are intrested
    // in ages up to 10 minutes or so.
    // The scaled values should suit presentation as opacity value.
    if(scaledAge < 1) { scaledAge = 1; }
    if(scaledAge > 600) { scaledAge = 600; }
    qDebug() << "Dog position age:" << scaledAge;
    return scaledAge;
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
        if(jsonObj["version"].toString() == "0.2") {
            m_currentDogPosition.setLatitude(jsonObj["lat"].toString().toDouble());
            m_currentDogPosition.setLongitude(jsonObj["lon"].toString().toDouble());
            m_dogPositionAccuracy = jsonObj["accuracy"].toString().toDouble();
            m_dogPositionAge = jsonObj["age"].toString().toInt();
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

#ifndef DOGTRACKER_H
#define DOGTRACKER_H

#include <QObject>
#include <QGeoPositionInfoSource>
#include <QTimer>

class DogTracker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int dogTrackingInterval READ dogTrackingInterval
                WRITE setdogTrackingInterval NOTIFY dogTrackingIntervalChanged)
    Q_PROPERTY(QGeoCoordinate currentDogPosition READ currentDogPosition
                NOTIFY currentDogPositionChanged)
    Q_PROPERTY(int dogPositionAccuracy READ dogPositionAccuracy NOTIFY currentDogPositionChanged)
    Q_PROPERTY(int dogPositionAge READ dogPositionAge NOTIFY currentDogPositionChanged)

public:
    explicit DogTracker(QObject *parent = 0);
    ~DogTracker();
    int dogTrackingInterval() const;
    void setdogTrackingInterval(int dogTrackingInterval);
    QGeoCoordinate currentDogPosition() const;
    int dogPositionAccuracy() const;
    int dogPositionAge() const;

signals:
    void dogTrackingIntervalChanged();
    void currentDogPositionChanged();

public slots:
    void fetchDogPosition();

private:
    QGeoCoordinate m_currentDogPosition;
    int m_dogPositionAccuracy;
    int m_dogPositionAge;
    int m_dogTrackingInterval;
    QTimer m_dogTrackingTimer;
    void requestDogPosition();

};

#endif // DOGTRACKER_H

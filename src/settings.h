/*
    Copyright 2014 Simo Mattila
    simo.h.mattila@gmail.com

    This file is part of Rena.

    Rena is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rena is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rena.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int updateInterval READ updateInterval
               WRITE setUpdateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(int dogTrackingInterval READ dogTrackingInterval
               WRITE setDogTrackingInterval NOTIFY dogTrackingIntervalChanged)
    Q_PROPERTY(QString dogTrackingUrl READ dogTrackingUrl
               WRITE setDogTrackingUrl NOTIFY dogTrackingUrlChanged)
    Q_PROPERTY(QString dogTrackingPassword READ dogTrackingPassword
               WRITE setDogTrackingPassword NOTIFY dogTrackingPasswordChanged)

public:
    explicit Settings(QObject *parent = 0);
    int updateInterval() const;
    void setUpdateInterval(int updateInterval);
    int dogTrackingInterval() const;
    void setDogTrackingInterval(int dogTrackingInterval);
    QString dogTrackingUrl() const;
    void setDogTrackingUrl(QString dogTrackingUrl);
    QString dogTrackingPassword() const;
    void setDogTrackingPassword(QString dogTrackingPassword);

signals:
    void updateIntervalChanged();
    void dogTrackingIntervalChanged();
    void dogTrackingUrlChanged();
    void dogTrackingPasswordChanged();

public slots:

private:
    QSettings *m_settings;
};

#endif // SETTINGS_H

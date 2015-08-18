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

#include "settings.h"

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    m_settings = new QSettings("Simom", "Rena");
}

int Settings::updateInterval() const {
    return m_settings->value("positioning/updateInterval", 1000).toInt();
}

void Settings::setUpdateInterval(int updateInterval) {
    m_settings->setValue("positioning/updateInterval", updateInterval);
    emit updateIntervalChanged();
}

int Settings::dogTrackingInterval() const {
    return m_settings->value("positioning/dogTrackingInterval", 0).toInt();
}

void Settings::setDogTrackingInterval(int dogTrackingInterval) {
    m_settings->setValue("positioning/dogTrackingInterval", dogTrackingInterval);
    emit dogTrackingIntervalChanged();
}

QString Settings::dogTrackingUrl() const {
    return m_settings->value("positioning/dogTrackingUrl", "").toString();
}

void Settings::setDogTrackingUrl(QString dogTrackingUrl) {
    m_settings->setValue("positioning/dogTrackingUrl", dogTrackingUrl);
    emit dogTrackingUrlChanged();
}

QString Settings::dogTrackingPassword() const {
    return m_settings->value("positioning/dogTrackingPassword", "").toString();
}

void Settings::setDogTrackingPassword(QString dogTrackingPassword) {
    m_settings->setValue("positioning/dogTrackingPassword", dogTrackingPassword);
    emit dogTrackingPasswordChanged();
}

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

#include <QStandardPaths>
#include <QDir>
#include <QSaveFile>
#include <QXmlStreamWriter>
#include <QDebug>
#include <qmath.h>
#include "trackrecorder.h"

TrackRecorder::TrackRecorder(QObject *parent) :
    QObject(parent)
{
    qDebug()<<"TrackRecorder constructor";
    m_distance = 0.0;
    m_accuracy = -1;
    m_tracking = false;
    m_isEmpty = true;
    m_applicationActive = true;
    m_autoSavePosition = 0;

    // Load autosaved track if left from previous session
    loadAutoSave();

    // Setup periodic autosave
    m_autoSaveTimer.setInterval(60000);
    connect(&m_autoSaveTimer, SIGNAL(timeout()), this, SLOT(autoSave()));
    m_autoSaveTimer.start();

    m_posSrc = QGeoPositionInfoSource::createDefaultSource(0);
    if (m_posSrc) {
        m_posSrc->setUpdateInterval(1000);
        connect(m_posSrc, SIGNAL(positionUpdated(QGeoPositionInfo)),
                this, SLOT(positionUpdated(QGeoPositionInfo)));
        connect(m_posSrc, SIGNAL(error(QGeoPositionInfoSource::Error)),
                this, SLOT(positioningError(QGeoPositionInfoSource::Error)));
        // Position updates are started/stopped in setIsTracking(...)
    } else {
        qDebug()<<"Failed initializing PositionInfoSource!";
    }
}

TrackRecorder::~TrackRecorder() {
    qDebug()<<"TrackRecorder destructor";
    autoSave();
}

void TrackRecorder::positionUpdated(const QGeoPositionInfo &newPos) {
    if(newPos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
        m_accuracy = newPos.attribute(QGeoPositionInfo::HorizontalAccuracy);
    } else {
        m_accuracy = -1;
    }
    emit accuracyChanged();

    m_currentPosition = newPos.coordinate();
    emit currentPositionChanged();

    if(newPos.hasAttribute(QGeoPositionInfo::HorizontalAccuracy) &&
            (newPos.attribute(QGeoPositionInfo::HorizontalAccuracy) > 30.0)) {
        return;
    }

    if(m_tracking) {
        m_points.append(newPos);
        emit pointsChanged();
        emit timeChanged();
        if(m_isEmpty) {
            m_isEmpty = false;
            m_minLat = m_maxLat = newPos.coordinate().latitude();
            m_minLon = m_maxLon = newPos.coordinate().longitude();
            emit isEmptyChanged();
        }

        if(m_points.size() > 1) {
            // Next line triggers following compiler warning?
            // \usr\include\qt5\QtCore\qlist.h:452: warning: assuming signed overflow does not occur when assuming that (X - c) > X is always false [-Wstrict-overflow]
            m_distance += m_points.at(m_points.size()-2).coordinate().distanceTo(m_points.at(m_points.size()-1).coordinate());
            emit distanceChanged();
            if(newPos.coordinate().latitude() < m_minLat) {
                m_minLat = newPos.coordinate().latitude();
            } else if(newPos.coordinate().latitude() > m_maxLat) {
                m_maxLat = newPos.coordinate().latitude();
            }
            if(newPos.coordinate().longitude() < m_minLon) {
                m_minLon = newPos.coordinate().longitude();
            } else if(newPos.coordinate().longitude() > m_maxLon) {
                m_maxLon = newPos.coordinate().longitude();
            }
        }
        emit newTrackPoint(newPos.coordinate());
    }
}

void TrackRecorder::positioningError(QGeoPositionInfoSource::Error error) {
    qDebug()<<"Positioning error:"<<error;
}

void TrackRecorder::exportGpx(QString name, QString desc, QString type) {
    qDebug()<<"Exporting track to gpx";
    if(m_points.size() < 1) {
        qDebug()<<"Nothing to save";
        return; // Nothing to save
    }
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString subDir = "Rena";
    QString filename;
    if(!name.isEmpty()) {
        filename = m_points.at(0).timestamp().toUTC().toString(Qt::ISODate)
                + " - " + name + ".gpx";
    } else {
        filename = m_points.at(0).timestamp().toUTC().toString(Qt::ISODate)
                + ".gpx";
    }
    qDebug()<<"File:"<<homeDir<<"/"<<subDir<<"/"<<filename;

    QDir home = QDir(homeDir);
    if(!home.exists(subDir)) {
        qDebug()<<"Directory does not exist, creating";
        if(home.mkdir(subDir)) {
            qDebug()<<"Directory created";
        } else {
            qDebug()<<"Directory creation failed, aborting";
            return;
        }
    }

    QSaveFile file;
    file.setFileName(homeDir + "/" + subDir + "/" + filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug()<<"File opening failed, aborting";
        return;
    }

    QXmlStreamWriter xml;
    xml.setDevice(&file);
    xml.setAutoFormatting(true);    // Human readable output
    xml.writeStartDocument();
    xml.writeDefaultNamespace("http://www.topografix.com/GPX/1/1");
    xml.writeStartElement("gpx");
    xml.writeAttribute("version", "1.1");
    xml.writeAttribute("Creator", "Rena for Sailfish");

    // Now this is a bit redundant, but to be able to write the totals in the metadata section we do need to loop
    // through the whole pointset here...
    qreal duration = 0;
    qreal distance = 0;
    qreal avg_speed = 0;
    qreal max_speed = 0;
    QDateTime start_time(m_points.at(0).timestamp());
    QDateTime stop_time(m_points.at(m_points.size()-1).timestamp());
    duration = start_time.secsTo(stop_time);
    for(int i=1 ; i < m_points.size(); i++) {
        if(m_points.at(i-1).coordinate().type() != QGeoCoordinate::Coordinate3D) {
            break; // No position info, skip this point
        }
        QGeoCoordinate first(m_points.at(i-1).coordinate().latitude(), m_points.at(i-1).coordinate().longitude());
        QGeoCoordinate second(m_points.at(i).coordinate().latitude(), m_points.at(i).coordinate().longitude());
        distance += first.distanceTo(second);
        if(m_points.at(i).attribute(QGeoPositionInfo::GroundSpeed) > max_speed) {
            max_speed = m_points.at(i).attribute(QGeoPositionInfo::GroundSpeed);
        }
    }
    if(distance == 0 || duration == 0) {
        avg_speed = 0;
    } else {
        avg_speed = distance / duration;
    }

    xml.writeStartElement("metadata");
    if(!name.isEmpty()) {
        xml.writeTextElement("name", name);
    }
    if(!desc.isEmpty()) {
        xml.writeTextElement("desc", desc);
    }
    if(!type.isEmpty()) {
        xml.writeTextElement("type", type);
    }
    xml.writeTextElement("duration", QString::number(duration));
    xml.writeTextElement("distance", QString::number(distance));
    xml.writeTextElement("avg_speed", QString::number(avg_speed));
    xml.writeTextElement("max_speed", QString::number(max_speed));
    xml.writeEndElement(); // metadata

    xml.writeStartElement("trk");
    xml.writeStartElement("trkseg");

    for(int i=0 ; i < m_points.size(); i++) {
        if(m_points.at(i).coordinate().type() == QGeoCoordinate::InvalidCoordinate) {
            break; // No position info, skip this point
        }
        xml.writeStartElement("trkpt");
        xml.writeAttribute("lat", QString::number(m_points.at(i).coordinate().latitude(), 'g', 15));
        xml.writeAttribute("lon", QString::number(m_points.at(i).coordinate().longitude(), 'g', 15));

        xml.writeTextElement("time", m_points.at(i).timestamp().toUTC().toString(Qt::ISODate));
        if(m_points.at(i).coordinate().type() == QGeoCoordinate::Coordinate3D) {
            xml.writeTextElement("ele", QString::number(m_points.at(i).coordinate().altitude(), 'g', 15));
        }

        xml.writeStartElement("extensions");
        if(m_points.at(i).hasAttribute(QGeoPositionInfo::Direction)) {
            xml.writeTextElement("dir", QString::number(m_points.at(i).attribute(QGeoPositionInfo::Direction), 'g', 15));
        }
        if(m_points.at(i).hasAttribute(QGeoPositionInfo::GroundSpeed)) {
            xml.writeTextElement("g_spd", QString::number(m_points.at(i).attribute(QGeoPositionInfo::GroundSpeed), 'g', 15));
        }
        if(m_points.at(i).hasAttribute(QGeoPositionInfo::VerticalSpeed)) {
            xml.writeTextElement("v_spd", QString::number(m_points.at(i).attribute(QGeoPositionInfo::VerticalSpeed), 'g', 15));
        }
        if(m_points.at(i).hasAttribute(QGeoPositionInfo::MagneticVariation)) {
            xml.writeTextElement("m_var", QString::number(m_points.at(i).attribute(QGeoPositionInfo::MagneticVariation), 'g', 15));
        }
        if(m_points.at(i).hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
            xml.writeTextElement("h_acc", QString::number(m_points.at(i).attribute(QGeoPositionInfo::HorizontalAccuracy), 'g', 15));
        }
        if(m_points.at(i).hasAttribute(QGeoPositionInfo::VerticalAccuracy)) {
            xml.writeTextElement("v_acc", QString::number(m_points.at(i).attribute(QGeoPositionInfo::VerticalAccuracy), 'g', 15));
        }
        xml.writeEndElement(); // extensions

        xml.writeEndElement(); // trkpt
    }

    xml.writeEndElement(); // trkseg
    xml.writeEndElement(); // trk

    xml.writeEndElement(); // gpx
    xml.writeEndDocument();

    file.commit();
    if(file.error()) {
        qDebug()<<"Error in writing to a file";
        qDebug()<<file.errorString();
    } else {
        QDir renaDir = QDir(homeDir + "/" + subDir);
        renaDir.remove("Autosave");
    }
}

void TrackRecorder::clearTrack() {
    m_points.clear();
    m_distance = 0;
    m_isEmpty = true;

    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString subDir = "Rena";
    QDir renaDir = QDir(homeDir + "/" + subDir);
    renaDir.remove("Autosave");

    emit distanceChanged();
    emit timeChanged();
    emit isEmptyChanged();
    emit pointsChanged();
}

qreal TrackRecorder::accuracy() const {
    return m_accuracy;
}

int TrackRecorder::points() const {
    return m_points.size();
}

qreal TrackRecorder::distance() const {
    return m_distance;
}

QString TrackRecorder::time() const {
    uint hours, minutes, seconds;

    if(m_points.size() < 2) {
        hours = 0;
        minutes = 0;
        seconds = 0;
    } else {
        QDateTime first = m_points.at(0).timestamp();
        QDateTime last = m_points.at(m_points.size()-1).timestamp();
        qint64 difference = first.secsTo(last);
        hours = difference / (60*60);
        minutes = (difference - hours*60*60) / 60;
        seconds = difference - hours*60*60 - minutes*60;
    }

    QString timeStr = QString("%1h %2m %3s")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));

    return timeStr;
}

bool TrackRecorder::isTracking() const {
    return m_tracking;
}

void TrackRecorder::setIsTracking(bool tracking) {
    if(m_tracking == tracking) {
        return; // No change
    }
    m_tracking = tracking;

    if(m_posSrc) {  // If we have positioning
        if(m_tracking && !m_applicationActive) {
            // Start tracking when application at background -> positioning has to be enabled
            m_posSrc->startUpdates();
        }
        if(!m_tracking && !m_applicationActive) {
            // Stop tracking when application at background -> disable positioning
            m_posSrc->stopUpdates();
        }
    }

    emit isTrackingChanged();
}

bool TrackRecorder::isEmpty() const {
    return m_isEmpty;
}

bool TrackRecorder::applicationActive() const {
    return m_applicationActive;
}

void TrackRecorder::setApplicationActive(bool active) {
    if(m_applicationActive == active) {
        return; // No change
    }
    m_applicationActive = active;

    if(m_posSrc) {  // If we have positioning
        if(m_applicationActive && !m_tracking) {
            // Application became active without tracking
            m_posSrc->startUpdates();
        }
        if(!m_applicationActive && !m_tracking) {
            // Application went to background without tracking
            m_posSrc->stopUpdates();
        }
    }

    emit applicationActiveChanged();
}

QGeoCoordinate TrackRecorder::currentPosition() const {
    return m_currentPosition;
}

int TrackRecorder::updateInterval() const {
    return m_posSrc->updateInterval();
}

void TrackRecorder::setUpdateInterval(int updateInterval) {
    if(!m_posSrc) {
        qDebug()<<"Can't set update interval, position source not initialized!";
        return;
    }
    m_posSrc->setUpdateInterval(updateInterval);
    qDebug()<<"Setting update interval to"<<updateInterval<<"msec";
    emit updateIntervalChanged();
}

QGeoCoordinate TrackRecorder::trackPointAt(int index) {
    if(index < m_points.length()) {
        return m_points.at(index).coordinate();
    } else {
        return QGeoCoordinate();
    }
}

int TrackRecorder::fitZoomLevel(int width, int height) {
    if(m_points.size() < 2 || width < 1 || height < 1) {
        // One point track or zero size map
        return 20;
    }

    // Keep also current position in view
    qreal minLon = qMin(m_minLon, (qreal)m_currentPosition.longitude());
    qreal maxLon = qMax(m_maxLon, (qreal)m_currentPosition.longitude());
    qreal minLat = qMin(m_minLat, (qreal)m_currentPosition.latitude());
    qreal maxLat = qMax(m_maxLat, (qreal)m_currentPosition.latitude());

    qreal trackMinX = (minLon + 180) / 360;
    qreal trackMaxX = (maxLon + 180) / 360;
    qreal trackMinY = sqrt(1-qLn(minLat*M_PI/180 + 1/qCos(minLat*M_PI/180))/M_PI);
    qreal trackMaxY = sqrt(1-qLn(maxLat*M_PI/180 + 1/qCos(maxLat*M_PI/180))/M_PI);

    qreal coord, pixel;
    qreal trackAR = qAbs((trackMaxX - trackMinX) / (trackMaxY - trackMinY));
    qreal windowAR = (qreal)width/(qreal)height;
    if(trackAR > windowAR ) {
        // Width limits
        coord = qAbs(trackMaxX - trackMinX);
        pixel = width;
    } else {
        // height limits
        coord = qAbs(trackMaxY - trackMinY);
        pixel = height;
    }

    // log2(x) = ln(x)/ln(2)
    int z = qFloor(qLn(pixel/256.0 * 1.0/coord * qCos((m_minLat+m_maxLat)/2*M_PI/180))
                   / qLn(2)) + 1;
    return z;
}

QGeoCoordinate TrackRecorder::trackCenter() {
    // Keep also current position in view
    qreal minLon = qMin(m_minLon, (qreal)m_currentPosition.longitude());
    qreal maxLon = qMax(m_maxLon, (qreal)m_currentPosition.longitude());
    qreal minLat = qMin(m_minLat, (qreal)m_currentPosition.latitude());
    qreal maxLat = qMax(m_maxLat, (qreal)m_currentPosition.latitude());

    return QGeoCoordinate((minLat+maxLat)/2, (minLon+maxLon)/2);
}

void TrackRecorder::autoSave() {
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString subDir = "Rena";
    QString filename = "Autosave";
    QDir home = QDir(homeDir);

    if(m_points.size() < 1) {
        // Nothing to save
        return;
    }

    qDebug()<<"Autosaving";

    if(!home.exists(subDir)) {
        qDebug()<<"Directory does not exist, creating";
        if(home.mkdir(subDir)) {
            qDebug()<<"Directory created";
        } else {
            qDebug()<<"Directory creation failed, aborting";
            return;
        }
    }
    QFile file;
    file.setFileName(homeDir + "/" + subDir + "/" + filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qDebug()<<"File opening failed, aborting";
        return;
    }
    QTextStream stream(&file);
    stream.setRealNumberPrecision(15);

    while(m_autoSavePosition < m_points.size()) {
        stream<<m_points.at(m_autoSavePosition).coordinate().latitude();
        stream<<" ";
        stream<<m_points.at(m_autoSavePosition).coordinate().longitude();
        stream<<" ";
        stream<<m_points.at(m_autoSavePosition).timestamp().toUTC().toString(Qt::ISODate);
        stream<<" ";
        if(m_points.at(m_autoSavePosition).coordinate().type() == QGeoCoordinate::Coordinate3D) {
            stream<<m_points.at(m_autoSavePosition).coordinate().altitude();
            stream<<" ";
        } else {
            stream<<"nan ";
        }
        stream<<m_points.at(m_autoSavePosition).attribute(QGeoPositionInfo::Direction);
        stream<<" ";
        stream<<m_points.at(m_autoSavePosition).attribute(QGeoPositionInfo::GroundSpeed);
        stream<<" ";
        stream<<m_points.at(m_autoSavePosition).attribute(QGeoPositionInfo::VerticalSpeed);
        stream<<" ";
        stream<<m_points.at(m_autoSavePosition).attribute(QGeoPositionInfo::MagneticVariation);
        stream<<" ";
        stream<<m_points.at(m_autoSavePosition).attribute(QGeoPositionInfo::HorizontalAccuracy);
        stream<<" ";
        stream<<m_points.at(m_autoSavePosition).attribute(QGeoPositionInfo::VerticalAccuracy);
        stream<<'\n';
        m_autoSavePosition++;
    }
    stream.flush();
    file.close();
}

void TrackRecorder::loadAutoSave() {
    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString subDir = "Rena";
    QString filename = "Autosave";
    QFile file;
    file.setFileName(homeDir + "/" + subDir + "/" + filename);
    if(!file.exists()) {
        qDebug()<<"No autosave found";
        return;
    }

    qDebug()<<"Loading autosave";

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"File opening failed, aborting";
        return;
    }
    QTextStream stream(&file);

    while(!stream.atEnd()) {
        QGeoPositionInfo point;
        qreal lat, lon, alt, temp;
        QString timeStr;
        stream>>lat>>lon>>timeStr>>alt;
        point.setCoordinate(QGeoCoordinate(lat, lon, alt));
        point.setTimestamp(QDateTime::fromString(timeStr,Qt::ISODate));
        stream>>temp;
        if(temp == temp) {  // If value is not nan
            point.setAttribute(QGeoPositionInfo::Direction, temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setAttribute(QGeoPositionInfo::GroundSpeed, temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setAttribute(QGeoPositionInfo::VerticalSpeed, temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setAttribute(QGeoPositionInfo::MagneticVariation, temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setAttribute(QGeoPositionInfo::HorizontalAccuracy, temp);
        }
        stream>>temp;
        if(temp == temp) {
            point.setAttribute(QGeoPositionInfo::VerticalAccuracy, temp);
        }
        stream.readLine(); // Read rest of the line, if any
        m_points.append(point);
        if(m_points.size() > 1) {
            if(point.coordinate().latitude() < m_minLat) {
                m_minLat = point.coordinate().latitude();
            } else if(point.coordinate().latitude() > m_maxLat) {
                m_maxLat = point.coordinate().latitude();
            }
            if(point.coordinate().longitude() < m_minLon) {
                m_minLon = point.coordinate().longitude();
            } else if(point.coordinate().longitude() > m_maxLon) {
                m_maxLon = point.coordinate().longitude();
            }
        } else {
            m_minLat = m_maxLat = point.coordinate().latitude();
            m_minLon = m_maxLon = point.coordinate().longitude();
        }
        emit newTrackPoint(point.coordinate());
    }
    m_autoSavePosition = m_points.size();
    file.close();

    qDebug()<<m_autoSavePosition<<"track points loaded";

    emit pointsChanged();
    emit timeChanged();

    if(m_points.size() > 1) {
        for(int i=1;i<m_points.size();i++) {
            m_distance += m_points.at(i-1).coordinate().distanceTo(m_points.at(i).coordinate());
        }
        emit distanceChanged();
    }

    if(!m_points.isEmpty()) {
        m_isEmpty = false;
        emit isEmptyChanged();
    }
}

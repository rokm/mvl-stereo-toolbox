/*
 * DC1394 Image Source: camera list model
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef CAMERA_LIST_MODEL_H
#define CAMERA_LIST_MODEL_H

#include <QtCore>
#include <unicap.h>
#include <dc1394/dc1394.h>


class CameraListModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    CameraListModel (QObject * = 0);
    virtual ~CameraListModel ();

    void setDeviceList (const QVector<unicap_device_t> &);
    
    void setActive (int, bool);
    void setActive (const unicap_device_t &, bool);

    const unicap_device_t &getDeviceInfo (int) const;

    // Model
    virtual int rowCount (const QModelIndex &) const;
    virtual Qt::ItemFlags flags (const QModelIndex &index) const;
    virtual QVariant data (const QModelIndex &index, int role) const;

protected:
    QVector<unicap_device_t> entries;
    QVector<bool> active;
};


#endif

/*
 * DC1394 Image Source: camera list model
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifndef CAMERA_LIST_MODEL_H
#define CAMERA_LIST_MODEL_H

#include <QtCore>
#include <dc1394/dc1394.h>


class CameraListModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    CameraListModel (QObject * = 0);
    virtual ~CameraListModel ();

    void setDeviceList (const dc1394camera_list_t *);
    
    void setActive (int, bool);
    void setActive (const dc1394camera_id_t &, bool);

    const dc1394camera_id_t &getDeviceId (int) const;

    // Model
    virtual int rowCount (const QModelIndex &) const;
    virtual Qt::ItemFlags flags (const QModelIndex &index) const;
    virtual QVariant data (const QModelIndex &index, int role) const;

protected:
    QList<dc1394camera_id_t> entries;
    QVector<bool> active;
};


#endif

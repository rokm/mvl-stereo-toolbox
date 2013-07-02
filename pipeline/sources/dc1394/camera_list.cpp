/*
 * DC1394 Image Source: camera model
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

#include "camera_list.h"

using namespace SourceDC1394;


CameraList::CameraList (QObject *parent)
    : QAbstractListModel(parent)
{
}

CameraList::~CameraList ()
{
}

void CameraList::setDeviceList (const dc1394camera_list_t *list)
{
    // Clear old entries
    if (entries.size()) {
        beginRemoveRows(QModelIndex(), 1, entries.size()); // Remove all but first entry, which is "None"
        entries.clear();
        endRemoveRows();
    }

    if (list->num) {
        beginInsertRows(QModelIndex(), 1, list->num);
        for (unsigned int i = 0; i < list->num; i++) {
            entries.append(list->ids[i]);
        }
        active = QVector<bool>(list->num, false);
        endInsertRows();
    }
}


const dc1394camera_id_t &CameraList::getDeviceId (int c) const
{
    return entries[c];
}


void CameraList::setActive (int c, bool value)
{
    active[c] = value;

    // Emit data changed
    dataChanged(index(c+1, 0), index(c+1, 0));
}

void CameraList::setActive (const dc1394camera_id_t &id, bool value)
{
    for (int i = 0; i < entries.size(); i++) {
        const dc1394camera_id_t &storedId = entries[i];
        if (storedId.guid == id.guid && storedId.unit == id.unit) {
            setActive(i, value);
            break;
        }
    }
}


// *********************************************************************
// *                               Model                               *
// *********************************************************************
int CameraList::rowCount (const QModelIndex &) const
{
    return entries.size() + 1;
}

Qt::ItemFlags CameraList::flags (const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (index.row() == 0) {
        // Always selectable
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    } else {
        int i = index.row() - 1;
        // Selectable only if not enabled
        if (!active[i]) {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        } else {
            return Qt::NoItemFlags;
        }
    }
}

QVariant CameraList::data (const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    // First element is always "None"
    if (index.row() == 0) {
        switch (role) {
            case Qt::DisplayRole: {
                return "None";
            }
            case Qt::ToolTipRole: {
                return "No device selected";
            }
            default: {
                return QVariant();
            }
        }

        return QVariant();
    }

    // Other valid devices
    const dc1394camera_id_t &entry = entries[index.row() - 1];
    switch (role) {
        case Qt::DisplayRole: {
            return QString("%1:%2").arg(entry.guid, 8, 16, QChar('0')).arg(entry.unit);
        }
        case Qt::UserRole: {
            // Index of device
            return index.row() - 1;
        }
    }

    return QVariant();
}

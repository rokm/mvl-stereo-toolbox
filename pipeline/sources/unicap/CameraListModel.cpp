/*
 * Unicap Image Source: camera list model
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

#include "CameraListModel.h"


CameraListModel::CameraListModel (QObject *parent)
    : QAbstractListModel(parent)
{
}

CameraListModel::~CameraListModel()
{
}

void CameraListModel::setDeviceList (const QVector<unicap_device_t> &list)
{
    beginResetModel();
    entries = list;
    active.resize(list.size());
    endResetModel();
}


void CameraListModel::setActive (int c, bool value)
{
    active[c] = value;

    // Emit data changed
    dataChanged(index(c+1, 0), index(c+1, 0));
}

void CameraListModel::setActive (const unicap_device_t &dev, bool value)
{
    for (int i = 0; i < entries.size(); i++) {
        if (!strcmp(entries[i].identifier, dev.identifier)) {
            setActive(i, value);
            break;
        }
    }
}

const unicap_device_t &CameraListModel::getDeviceInfo (int c) const
{
    return entries[c];
}


// *********************************************************************
// *                               Model                               *
// *********************************************************************
int CameraListModel::rowCount (const QModelIndex &) const
{
    return entries.size() + 1;
}

Qt::ItemFlags CameraListModel::flags (const QModelIndex &index) const
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

QVariant CameraListModel::data (const QModelIndex &index, int role) const
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
    const unicap_device_t &entry = entries[index.row() - 1];
    switch (role) {
        case Qt::DisplayRole: {
            return QString("%1").arg(entry.identifier);
        }
        case Qt::UserRole: {
            // Index of device
            return index.row() - 1;
        }
    }

    return QVariant();
}

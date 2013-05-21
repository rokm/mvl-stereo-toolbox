#include "CameraListModel.h"


CameraListModel::CameraListModel (QObject *parent)
    : QAbstractListModel(parent)
{
}

CameraListModel::~CameraListModel()
{
}

void CameraListModel::setDeviceList (const dc1394camera_list_t *list)
{
    beginResetModel();
    
    entries.clear();
    for (int i = 0; i < list->num; i++) {
        entries.append(list->ids[i]);
    }
    active.resize(list->num);

    endResetModel();
}

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

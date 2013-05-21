#ifndef CAMERA_LIST_MODEL
#define CAMERA_LIST_MODEL

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

    const dc1394camera_id_t &getDeviceInfo (int) const;

    // Model
    virtual int rowCount (const QModelIndex &) const;
    virtual Qt::ItemFlags flags (const QModelIndex &index) const;
    virtual QVariant data (const QModelIndex &index, int role) const;

protected:
    QList<dc1394camera_id_t> entries;
    QVector<bool> active;
};


#endif

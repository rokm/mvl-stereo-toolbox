#ifndef IMAGE_SOURCE_DC1394_H
#define IMAGE_SOURCE_DC1394_H

#include "ImageSource.h"

#include <dc1394/dc1394.h>


class CameraDC1394;
class CameraListModel;


class ImageSourceDC1394 : public ImageSource
{
    Q_OBJECT

public:
    ImageSourceDC1394 (QObject * = 0);
    virtual ~ImageSourceDC1394 ();

    CameraListModel *getCameraListModel();

    void setLeftCamera (int);
    void setRightCamera (int);

    CameraDC1394 *getLeftCamera ();
    CameraDC1394 *getRightCamera ();

public slots:
    void scanBus ();
    void startStopCapture (bool);

protected:
    void createCamera (CameraDC1394 *&, int);
    void releaseCamera (CameraDC1394 *&);

protected slots:
    void frameAggregator ();

signals:
    void leftCameraChanged ();
    void rightCameraChanged ();

protected:
    dc1394_t *fw;
    
    CameraListModel *cameraListModel;

    CameraDC1394 *leftCamera;
    CameraDC1394 *rightCamera;

    bool leftFrameReady, rightFrameReady;
};

// Config widget
#include <QtGui>

class ConfigTabDC1394 : public QWidget
{
    Q_OBJECT
    
public:
    ConfigTabDC1394 (ImageSourceDC1394 *, QWidget * = 0);
    virtual ~ConfigTabDC1394 ();

protected slots:
    void deviceSelected (int);
    void startStopCapture (bool);
    
protected:
    QWidget *createDeviceFrame (bool);

    void deviceSelected (QWidget *&, QFrame *&, QComboBox *&, int);
    void startStopCapture (CameraDC1394 *, bool);

protected:
    ImageSourceDC1394 *source;

    // GUI
    QComboBox *comboBoxLeftDevice;
    QComboBox *comboBoxRightDevice;

    QPushButton *pushButtonCaptureLeftDevice;
    QPushButton *pushButtonCaptureRightDevice;

    QHBoxLayout *boxDevices;
    
    QFrame *frameLeftDevice;
    QFrame *frameRightDevice;

    QWidget *configLeftDevice;
    QWidget *configRightDevice;
};

#endif

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

public slots:
    void scanBus ();
    void startStopCapture (bool);

protected:
    void createCamera (CameraDC1394 *&, int);
    void releaseCamera (CameraDC1394 *&);

protected slots:
    void captureFunction ();

signals:
    void captureFunctionFinished ();

protected:
    dc1394_t *fw;
    
    CameraListModel *cameraListModel;

    CameraDC1394 *leftCamera;
    CameraDC1394 *rightCamera;

    QThread *captureThread;
    bool captureActive;
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
    void cameraLeftSelected (int);
    void cameraRightSelected (int);

protected:
    ImageSourceDC1394 *source;

    // GUI
    QComboBox *comboBoxLeftDevice;
    QComboBox *comboBoxRightDevice;
};

#endif

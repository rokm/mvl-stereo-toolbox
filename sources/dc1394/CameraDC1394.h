#ifndef CAMERA_DC1394_H
#define CAMERA_DC1394_H

#include <QtCore>
#include <QtGui>
#include <dc1394/dc1394.h>

#include <opencv2/core/core.hpp>

class ConfigCameraDC1394;


class CameraDC1394 : public QObject
{
    Q_OBJECT

public:
    CameraDC1394 (dc1394camera_t *, QObject * = 0);
    virtual ~CameraDC1394 ();

    // Config widget
    QWidget *getConfigWidget ();

    // Camera identification
    dc1394camera_id_t getId () const;
    bool isSameCamera (const dc1394camera_id_t &) const;

    QString getVendor () const;
    QString getModel () const;

    // Basic parameters    
    void setIsoSpeed (dc1394speed_t);
    dc1394speed_t getIsoSpeed () const;

    QVector<dc1394video_mode_t> getSupportedModes ();
    void setMode (dc1394video_mode_t);
    dc1394video_mode_t getMode () const;

    QVector<dc1394framerate_t> getSupportedFramerates ();
    void setFramerate (dc1394framerate_t);
    dc1394framerate_t getFramerate () const;

    // Camera start/stop
    void startCamera ();
    void stopCamera ();

    // Advanced grabbing interface
    void dequeueCaptureBuffer (dc1394video_frame_t *&, bool = false);
    void enqueueCaptureBuffer (dc1394video_frame_t *);
    void convertToOpenCVImage (dc1394video_frame_t *, cv::Mat &) const;

    // Simplified grabbing interface
    void grabFrame (cv::Mat &);

protected:
    dc1394camera_id_t id;
    
    dc1394camera_t *camera;
    dc1394featureset_t features;

    // Config widget
    ConfigCameraDC1394 *configWidget;
};


class ConfigCameraDC1394 : public QWidget
{
    Q_OBJECT
    
public:
    ConfigCameraDC1394 (CameraDC1394 *, QWidget * = 0);
    virtual ~ConfigCameraDC1394 ();

protected:
    CameraDC1394 *camera;
};


#endif

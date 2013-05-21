#ifndef CAMERA_DC1394_H
#define CAMERA_DC1394_H

#include <QtCore>
#include <dc1394/dc1394.h>

#include <opencv2/core/core.hpp>


class CameraDC1394 : public QObject
{
    Q_OBJECT

public:
    CameraDC1394 (dc1394camera_t *, QObject * = 0);
    virtual ~CameraDC1394 ();

    // Camera identification
    dc1394camera_id_t getId () const;
    bool isSameCamera (const dc1394camera_id_t &) const;
    
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
};

#endif

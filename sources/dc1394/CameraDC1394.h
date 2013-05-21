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

    void setIsoSpeed (dc1394speed_t);
    dc1394speed_t getIsoSpeed () const;
    
    void setMode (dc1394video_mode_t);
    dc1394video_mode_t getMode () const;
    
    void setFramerate (dc1394framerate_t);
    dc1394framerate_t getFramerate () const;
    
    void startCamera ();
    void stopCamera ();

    void grabFrame (cv::Mat &);
    
protected:
    dc1394camera_t *camera;
    dc1394featureset_t features;

    dc1394video_frame_t *frame;
};

#endif

#include "CameraDC1394.h"

#define NUM_BUFFERS 8


CameraDC1394::CameraDC1394 (dc1394camera_t *c, QObject *parent)
    : QObject(parent), camera(c)
{
    //setIsoSpeed();
    setMode(DC1394_VIDEO_MODE_1024x768_MONO8);
    setFramerate(DC1394_FRAMERATE_7_5);

    // Print info
    dc1394_camera_print_info(camera, stdout);
}

CameraDC1394::~CameraDC1394 ()
{
    qDebug() << "Destroying" << this;
    stopCamera();
}


dc1394camera_id_t CameraDC1394::getId () const
{
    dc1394camera_id_t id;
    id.guid = camera->guid;
    id.unit = camera->unit;
    return id;
}

bool CameraDC1394::isSameCamera (const dc1394camera_id_t &id) const
{
    return (id.guid == camera->guid) && (id.unit == camera->unit);
}


void CameraDC1394::setIsoSpeed (dc1394speed_t speed)
{
    dc1394error_t ret;

    ret = dc1394_video_set_iso_speed(camera, speed);
    if (ret) {
        qWarning() << "Could not set ISO speed!";
        return;
    }
}

dc1394speed_t CameraDC1394::getIsoSpeed () const
{
    dc1394speed_t value;
    dc1394error_t ret;

    // Set mode
    ret = dc1394_video_get_iso_speed(camera, &value);
    if (ret) {
        qWarning() << "Could not get ISO speed!";
    }
    
    return value;
}


void CameraDC1394::setMode (dc1394video_mode_t mode)
{
    dc1394error_t ret;

    // Set mode
    ret = dc1394_video_set_mode(camera, mode);
    if (ret) {
        qWarning() << "Could not set mode!";
        return;
    }
}

dc1394video_mode_t CameraDC1394::getMode () const
{
    dc1394video_mode_t value;
    dc1394error_t ret;

    // Set mode
    ret = dc1394_video_get_mode(camera, &value);
    if (ret) {
        qWarning() << "Could not get mode!";
    }
    
    return value;
}


void CameraDC1394::setFramerate (dc1394framerate_t fps)
{
    dc1394error_t ret;

    // Set framerate
    ret = dc1394_video_set_framerate(camera, fps);
    if (ret) {
        qWarning() << "Could not set framerate!";
        return;
    }
}

dc1394framerate_t CameraDC1394::getFramerate () const
{
    dc1394framerate_t value;
    dc1394error_t ret;

    // Set mode
    ret = dc1394_video_get_framerate(camera, &value);
    if (ret) {
        qWarning() << "Could not get framerate!";
    }
    
    return value;
}



void CameraDC1394::startCamera ()
{
    dc1394error_t ret;

    // Setup capture
    ret = dc1394_capture_setup(camera, NUM_BUFFERS, DC1394_CAPTURE_FLAGS_DEFAULT);
    if (ret) {
        qWarning() << "Could not setup camera! Make sure that the video mode and framerate are supported by the camera!";
        return;
    }

    // Start ISO transmission
    ret = dc1394_video_set_transmission(camera, DC1394_ON);
    if (ret) {
        qWarning() << "Could not start camera ISO transmission!";
        return;
    }
}

void CameraDC1394::stopCamera ()
{
    dc1394error_t ret;
    
    ret = dc1394_video_set_transmission(camera, DC1394_OFF);
    if (ret) {
        qWarning() << "Could not stop camera ISO transmission!";
        return;
    }
    
    ret = dc1394_capture_stop(camera);
    if (ret) {
        qWarning() << "Could not stop camera capture!";
        return;
    }
}



void CameraDC1394::dequeueCaptureBuffer (dc1394video_frame_t *&frame, bool drainQueue)
{
    dc1394error_t ret;
    
    // Dequeue
    ret = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
    if (ret) {
        qWarning() << "Could not dequeue frame!";
        return;
    }

    if (drainQueue) {
        // Drain until we're no frames behind
        while (frame->frames_behind) {
            qDebug() << "Draining capture queue due to being" << frame->frames_behind << "frames!";
            ret = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
            if (ret) {
                qWarning() << "Could not dequeue frame!";
                return;
            }
        }
    }
}

void CameraDC1394::enqueueCaptureBuffer (dc1394video_frame_t *frame)
{
    dc1394error_t ret;

    // Enqueue
    ret = dc1394_capture_enqueue (camera, frame);
    if (ret) {
        qWarning() << "Could not enque buffer!";
        return;
    }
}

void CameraDC1394::convertToOpenCVImage (dc1394video_frame_t *frame, cv::Mat &image) const
{
    cv::Mat tmpImg(frame->size[1], frame->size[0], CV_8UC1, frame->image);
    tmpImg.copyTo(image);
}


// Convenience function for grabbing from single camera
void CameraDC1394::grabFrame (cv::Mat &image)
{
    dc1394video_frame_t *frame;

    // Dequeue
    dequeueCaptureBuffer(frame);
    // Convert
    convertToOpenCVImage(frame, image);
    // Enqueue
    enqueueCaptureBuffer(frame);
}




#include "CameraDC1394.h"

#define NUM_BUFFERS 8


CameraDC1394::CameraDC1394 (dc1394camera_t *c, QObject *parent)
    : QObject(parent), camera(c)
{
    //setIsoSpeed();
    setMode(DC1394_VIDEO_MODE_1024x768_MONO8);
    setFramerate(DC1394_FRAMERATE_15);

    // Print info
    dc1394_camera_print_info(camera, stdout);

    // Config widget
    configWidget = new ConfigCameraDC1394(this);
}

CameraDC1394::~CameraDC1394 ()
{
    stopCamera();

    // Unparent the config widget and destroy it
    configWidget->setParent(0);
    delete configWidget;
}

QWidget *CameraDC1394::getConfigWidget ()
{
    return configWidget;
}


// *********************************************************************
// *                       Camera identification                       *
// *********************************************************************
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


QString CameraDC1394::getVendor () const
{
    return QString(camera->vendor);
}

QString CameraDC1394::getModel () const
{
    return QString(camera->model);
}


// *********************************************************************
// *                         Basic properties                          *
// *********************************************************************
// ISO speed
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


// Video mode
QVector<dc1394video_mode_t> CameraDC1394::getSupportedModes ()
{
    dc1394video_modes_t raw_modes;
    dc1394error_t ret;

    QVector<dc1394video_mode_t> modes; 
    
    ret = dc1394_video_get_supported_modes(camera, &raw_modes);
    if (ret) {
        qWarning() << "Failed to query supported modes!";
        return modes;
    }

    modes.resize(raw_modes.num);
    for (int i = 0; i < raw_modes.num; i++) {
        modes[i] = raw_modes.modes[i];
    }

    return modes;
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


// Framerate
QVector<dc1394framerate_t> CameraDC1394::getSupportedFramerates ()
{
    dc1394framerates_t raw_framerates;
    dc1394error_t ret;

    QVector<dc1394framerate_t> framerates; 
    
    ret = dc1394_video_get_supported_framerates(camera, getMode(), &raw_framerates);
    if (ret) {
        qWarning() << "Failed to query supported framerates!";
        return framerates;
    }

    framerates.resize(raw_framerates.num);
    for (int i = 0; i < raw_framerates.num; i++) {
        framerates[i] = raw_framerates.framerates[i];
    }

    return framerates;
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


// *********************************************************************
// *                         Camera start/stop                         *
// *********************************************************************
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


// *********************************************************************
// *                    Advanced grabbing interface                    *
// *********************************************************************
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
    // Determine if image is monochrome or color
    dc1394bool_t isColor = DC1394_FALSE;
    dc1394_is_color(frame->color_coding, &isColor);

    if (isColor == DC1394_TRUE) {
        // Convert into RGB8
        image.create(frame->size[1], frame->size[0], CV_8UC3); // (Re)allocate, if necessary
        dc1394_convert_to_RGB8(frame->image, image.ptr<uint8_t>(), frame->size[0], frame->size[1], frame->yuv_byte_order, frame->color_coding, frame->data_depth);
    } else {
        // Convert into Mono8
        image.create(frame->size[1], frame->size[0], CV_8UC1); // (Re)allocate, if necessary
        dc1394_convert_to_MONO8(frame->image, image.ptr<uint8_t>(), frame->size[0], frame->size[1], frame->yuv_byte_order, frame->color_coding, frame->data_depth);
    }
}


// *********************************************************************
// *                   Simplified grabbing interface                   *
// *********************************************************************
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


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigCameraDC1394::ConfigCameraDC1394 (CameraDC1394 *c, QWidget *parent)
    : QWidget(parent), camera(c)
{
    QFormLayout *layout = new QFormLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label;
    QComboBox *comboBox;
    QFrame *line;

    QString tooltip;

    connect(camera, SIGNAL(parameterChanged()), this, SLOT(updateParameters()));

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Vendor
    tooltip = "Camera vendor.";
    
    label = new QLabel("<b>Vendor: </b>" + camera->getVendor(), this);
    label->setToolTip(tooltip);

    layout->addRow(label);

    // Model
    tooltip = "Camera model.";
    
    label = new QLabel("<b>Model: </b>" + camera->getModel(), this);
    label->setToolTip(tooltip);

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);
}

ConfigCameraDC1394::~ConfigCameraDC1394 ()
{
}

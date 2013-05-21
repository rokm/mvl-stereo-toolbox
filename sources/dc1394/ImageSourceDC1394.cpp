#include "ImageSourceDC1394.h"

#include "CameraDC1394.h"
#include "CameraListModel.h"


ImageSourceDC1394::ImageSourceDC1394 (QObject *parent)
    : ImageSource(parent)
{
    fw = NULL;
    leftCamera = NULL;
    rightCamera = NULL;

    // Camera list
    cameraListModel = new CameraListModel(this);

    // Processing thread
    captureThread = new QThread(this);
    connect(captureThread, SIGNAL(started()), this, SLOT(captureFunction()), Qt::DirectConnection);
    connect(this, SIGNAL(captureFunctionFinished()), captureThread, SLOT(quit()));

    scanBus();

    // Config widget
    shortName = "DC1394";
    configWidget = new ConfigTabDC1394(this);
}

ImageSourceDC1394::~ImageSourceDC1394 ()
{
    if (fw) {
        dc1394_free(fw);
    }
}

CameraListModel *ImageSourceDC1394::getCameraListModel ()
{
    return cameraListModel;
}


void ImageSourceDC1394::scanBus ()
{
    dc1394error_t ret;

    // Release old cameras
    releaseCamera(leftCamera);
    releaseCamera(rightCamera);

    qDebug() << "Scanning bus for DC1394 devices...";
    
    // Create DC1394 object - if not already created
    if (!fw) {
        fw = dc1394_new();
    }
    if (!fw) {
        qWarning() << "Failed to create DC1394 object!";
        return;
    }

    // Enumerate cameras    
    dc1394camera_list_t *camera_list;

    ret = dc1394_camera_enumerate(fw, &camera_list);
    if (ret) {
        qWarning() << "Failed to enumerate cameras; error code:" << ret;
        return;
    }
    cameraListModel->setDeviceList(camera_list);
    dc1394_camera_free_list(camera_list);
    
}


static inline bool same_camera_id (const dc1394camera_id_t &id1, const dc1394camera_id_t &id2)
{
    return (id1.guid == id2.guid) && (id1.unit == id2.unit);
}



void ImageSourceDC1394::setLeftCamera (int c)
{
    // Create camera
    createCamera(leftCamera, c);
    emit leftCameraChanged();
}

void ImageSourceDC1394::setRightCamera (int c)
{
    // Create camera
    createCamera(rightCamera, c);
    emit rightCameraChanged();
}

void ImageSourceDC1394::createCamera (CameraDC1394 *& camera, int c)
{
    // If c is -1, release camera
    if (c == -1) {
        releaseCamera(camera);
        return;
    }

    // Get camera id from our list
    const dc1394camera_id_t &newId = cameraListModel->getDeviceId(c);

    // Check if it is the same as the current right camera
    if (rightCamera && rightCamera->isSameCamera(newId)) {
        qDebug() << "Same as current right!";
        return;
    }

    // Check if it is the same as the current left camera
    if (leftCamera && leftCamera->isSameCamera(newId)) {
        qDebug() << "Same as current left!";
        return;
    }

    // Release current camera
    releaseCamera(camera);

    // Create new camera
    dc1394camera_t *raw_camera = dc1394_camera_new_unit(fw, newId.guid, newId.unit);
    if (!raw_camera) {
        qWarning() << "Failed to create camera object!";
        return;
    }
    camera = new CameraDC1394(raw_camera, this);

    // Mark camera as active in our list
    cameraListModel->setActive(c, true);
}

void ImageSourceDC1394::releaseCamera (CameraDC1394 *& camera)
{
    if (camera) {
        dc1394camera_id_t id = camera->getId();

        // Delete camera object 
        delete camera;
        camera = NULL;

        // Mark camera as inactive in our list
        cameraListModel->setActive(id, false);
    }
}


CameraDC1394 *ImageSourceDC1394::getLeftCamera ()
{
    return leftCamera;
}

CameraDC1394 *ImageSourceDC1394::getRightCamera ()
{
    return rightCamera;
}


void ImageSourceDC1394::startStopCapture (bool start)
{
    if (start) {
        // Start the capture threa
        qDebug() << "Starting capture thread:" << QThread::currentThread();
        captureThread->start();
    } else {
        captureActive = false;
        // Do nothing else, the thread should finish itself
    }
}

void ImageSourceDC1394::captureFunction ()
{
    qDebug() << this << "Starting capture function in thread:" << QThread::currentThread();

    // Make sure at least one camera is valid
    if(!leftCamera && !rightCamera) {
        qWarning() << this << "At least one camera must be valid!";
        emit captureFunctionFinished();
        return;
    }

    // Start cameras
    if (leftCamera) leftCamera->startCamera();
    if (rightCamera) rightCamera->startCamera();

    // Frame pointers
    dc1394video_frame_t *frameLeft, *frameRight;

    // Capture loop
    QTime time; time.start();
    captureActive = true;
    while (captureActive) {
        // Dequeue frames - from both cameras, almost at once. Also, we
        // make sure to drain the queue, as we want the latest frames
        if (leftCamera) leftCamera->dequeueCaptureBuffer(frameLeft, true);
        if (rightCamera) rightCamera->dequeueCaptureBuffer(frameRight, true);

        // Convert to OpenCV... we copy directly into ImageSource's
        // images, therefore we need to do it under write lock
        QWriteLocker locker(&imagesLock);
        if (leftCamera) leftCamera->convertToOpenCVImage(frameLeft, imageLeft);
        if (rightCamera) rightCamera->convertToOpenCVImage(frameRight, imageRight);
        locker.unlock();

        // Signal that we have new images
        emit imagesChanged();

        // Equeue
        if (leftCamera) leftCamera->enqueueCaptureBuffer(frameLeft);
        if (rightCamera) rightCamera->enqueueCaptureBuffer(frameRight);
    }

    // Stop cameras
    if (leftCamera) {
        leftCamera->stopCamera();
    }
    if (rightCamera) {
        rightCamera->stopCamera();
    }

    qDebug() << this << "Finished capture function";
    emit captureFunctionFinished();
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabDC1394::ConfigTabDC1394 (ImageSourceDC1394 *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    configLeftDevice = NULL;
    configRightDevice = NULL;

    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QFrame *line, *frame;
    QPushButton *button;
    QComboBox *comboBox;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>DC1394 source</u></b>", this);
    label->setAlignment(Qt::AlignHCenter);
    
    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Rescan
    tooltip = "Rescan bus for DC1394 devices.";
    
    button = new QPushButton("Rescan");
    button->setToolTip(tooltip);
    connect(button, SIGNAL(released()), source, SLOT(scanBus()));

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Capture
    tooltip = "Start/stop capture.";
    
    button = new QPushButton("Capture");
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, SIGNAL(toggled(bool)), source, SLOT(startStopCapture(bool)));

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Cameras
    boxCameras = new QHBoxLayout();
    layout->addRow(boxCameras);

    createLeftCameraFrame();
    createRightCameraFrame();
}

void ConfigTabDC1394::createLeftCameraFrame ()
{
    QFrame *frame;
    QLabel *label;
    QComboBox *comboBox;
    QString tooltip;

    QFormLayout *layout;
   
    // Camera frame
    frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout = new QFormLayout(frame);
    frame->setLayout(layout);
    boxCameras->addWidget(frame);

    // Label
    label = new QLabel("<b>Left device</b>", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(label);

    // Combo box
    tooltip = "Left DC1394 device.";
    
    comboBox = new QComboBox(this);
    comboBox->setModel(source->getCameraListModel());
    comboBox->setToolTip(tooltip);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(cameraLeftSelected(int)));
    comboBoxLeftDevice = comboBox;
    layout->addRow(comboBox);

    // Camera config frame
    frame = new QFrame();
    frame->setLayout(new QVBoxLayout(frame));
    frame->setContentsMargins(0, 0, 0, 0);
    frame->layout()->setContentsMargins(0, 0, 0, 0);
    layout->addRow(frame);
    frameLeftDevice = frame;
}

void ConfigTabDC1394::createRightCameraFrame ()
{
    QFrame *frame;
    QLabel *label;
    QComboBox *comboBox;
    QString tooltip;

    QFormLayout *layout;
   
    // Camera frame
    frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout = new QFormLayout(frame);
    frame->setLayout(layout);
    boxCameras->addWidget(frame);

    // Label
    label = new QLabel("<b>Right device</b>", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(label);

    // Combo box
    tooltip = "Right DC1394 device.";
    
    comboBox = new QComboBox(this);
    comboBox->setModel(source->getCameraListModel());
    comboBox->setToolTip(tooltip);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(cameraRightSelected(int)));
    comboBoxRightDevice = comboBox;
    layout->addRow(comboBox);

    // Camera config frame
    frame = new QFrame();
    frame->setLayout(new QVBoxLayout(frame));
    frame->setContentsMargins(0, 0, 0, 0);
    frame->layout()->setContentsMargins(0, 0, 0, 0);
    layout->addRow(frame);
    frameRightDevice = frame;
}

ConfigTabDC1394::~ConfigTabDC1394 ()
{
}


void ConfigTabDC1394::cameraLeftSelected (int index)
{
    // Remove config widget for old device
    if (configLeftDevice) {
        frameLeftDevice->layout()->removeWidget(configLeftDevice);
    }
    
    // Set new device
    QVariant c = comboBoxLeftDevice->itemData(index);
    source->setLeftCamera(c.isValid() ? c.toInt() : -1);

    // Get new device's config widget
    CameraDC1394 *camera = source->getLeftCamera();
    if (camera) {
        configLeftDevice = camera->getConfigWidget();
        frameLeftDevice->layout()->addWidget(configLeftDevice);
    }
}

void ConfigTabDC1394::cameraRightSelected (int index)
{
    // Remove config widget for old device
    if (configRightDevice) {
        frameRightDevice->layout()->removeWidget(configRightDevice);
    }
    
    // Set new device
    QVariant c = comboBoxRightDevice->itemData(index);
    source->setRightCamera(c.isValid() ? c.toInt() : -1);

    // Get new device's config widget
    CameraDC1394 *camera = source->getRightCamera();
    if (camera) {
        configRightDevice = camera->getConfigWidget();
        frameRightDevice->layout()->addWidget(configRightDevice);
    }
}


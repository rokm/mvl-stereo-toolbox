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

    // Perform initial scan
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

    // Connect
    connect(camera, SIGNAL(frameReady()), this, SLOT(frameAggregator()));

    // Mark camera as active in our list
    cameraListModel->setActive(c, true);
}

void ImageSourceDC1394::releaseCamera (CameraDC1394 *& camera)
{
    if (camera) {
        dc1394camera_id_t id = camera->getId();

        // Disconnect
        disconnect(camera, SIGNAL(frameReady()), this, SLOT(frameAggregator()));

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
        leftFrameReady = rightFrameReady = false;
        if (leftCamera) leftCamera->startCapture();
        if (rightCamera) rightCamera->startCapture();
    } else {
        if (leftCamera) leftCamera->stopCapture();
        if (rightCamera) rightCamera->stopCapture();
    }
}

void ImageSourceDC1394::frameAggregator ()
{
    if (QObject::sender() == leftCamera) {
        leftFrameReady = true;
    } else if (QObject::sender() == rightCamera) {
        rightFrameReady = true;
    }
    
    bool requireLeft = (leftCamera && leftCamera->getCaptureState());
    bool requireRight = (rightCamera && rightCamera->getCaptureState());

    if ((!requireLeft || leftFrameReady) && (!requireRight || rightFrameReady)) {
        if (requireLeft) {
            leftCamera->copyFrame(imageLeft);
        } else {
            imageLeft = cv::Mat();
        }
        if (requireRight) {
            rightCamera->copyFrame(imageRight);
        } else {
            imageRight = cv::Mat();
        }
        leftFrameReady = false;
        rightFrameReady = false;

        emit imagesChanged();
    }
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

    // Devices
    boxDevices = new QHBoxLayout();
    layout->addRow(boxDevices);

    boxDevices->addWidget(createDeviceFrame(true)); // Left device frame
    boxDevices->addWidget(createDeviceFrame(false)); // Right device frame
}

QWidget *ConfigTabDC1394::createDeviceFrame (bool left)
{
    QFrame *deviceFrame, *frame;
    QLabel *label;
    QComboBox *comboBox;
    QPushButton *button;
    QFrame *line;
    QString tooltip;

    QFormLayout *layout;
   
    // Camera frame
    deviceFrame = new QFrame(this);
    deviceFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout = new QFormLayout(deviceFrame);
    deviceFrame->setLayout(layout);

    // Label
    label = new QLabel(left ? "<b>Left device</b>" : "<b>Right device</b>", deviceFrame);
    label->setAlignment(Qt::AlignCenter);
    layout->addRow(label);

    // Combo box
    tooltip = left ? "Left DC1394 device." : "Right DC1394 device.";
    
    comboBox = new QComboBox(deviceFrame);
    comboBox->setModel(source->getCameraListModel());
    comboBox->setToolTip(tooltip);
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(deviceSelected(int)));
    if (left) {
        comboBoxLeftDevice = comboBox;
    } else {
        comboBoxRightDevice = comboBox;
    }
    layout->addRow(comboBox);

    // Capture
    tooltip = "Start/stop capture.";
    
    button = new QPushButton("Capture", deviceFrame);
    button->setToolTip(tooltip);
    button->setCheckable(true);
    connect(button, SIGNAL(toggled(bool)), this, SLOT(startStopCapture(bool)));
    if (left) {
        pushButtonCaptureLeftDevice = button;
    } else {
        pushButtonCaptureRightDevice = button;
    }

    layout->addRow(button);

    // Separator
    line = new QFrame(deviceFrame);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Camera config frame
    frame = new QFrame(deviceFrame);
    frame->setLayout(new QVBoxLayout(frame));
    frame->setContentsMargins(0, 0, 0, 0);
    frame->layout()->setContentsMargins(0, 0, 0, 0);
    layout->addRow(frame);
    if (left) {
        frameLeftDevice = frame;
    } else {
        frameRightDevice = frame;
    }

    return deviceFrame;
}

ConfigTabDC1394::~ConfigTabDC1394 ()
{
}


void ConfigTabDC1394::deviceSelected (int index)
{
    QWidget **deviceConfig;
    QFrame **frame;
    QComboBox **comboBox;

    if (QObject::sender() == comboBoxLeftDevice) {
        deviceSelected(configLeftDevice, frameLeftDevice, comboBoxLeftDevice, index);
    } else if (QObject::sender() == comboBoxRightDevice) {
        deviceSelected(configRightDevice, frameRightDevice, comboBoxRightDevice, index);
    } else {
        qFatal("Invalid sender!");
    }
}

void ConfigTabDC1394::deviceSelected (QWidget *&deviceConfig, QFrame *&deviceFrame, QComboBox *&deviceComboBox, int index)
{
    
    // Remove config widget for old device
    if (deviceConfig) {
        deviceFrame->layout()->removeWidget(deviceConfig);
    }
    
    // Set new device
    CameraDC1394 *newDevice;
    QVariant c = deviceComboBox->itemData(index);
    
    if (deviceComboBox == comboBoxLeftDevice) {
        source->setLeftCamera(c.isValid() ? c.toInt() : -1);
        newDevice = source->getLeftCamera();
    } else {
        source->setRightCamera(c.isValid() ? c.toInt() : -1);
        newDevice = source->getRightCamera();
    }

    // Get new device's config widget
    if (newDevice) {
        deviceConfig = newDevice->getConfigWidget();
        deviceFrame->layout()->addWidget(deviceConfig);
    }
}

void ConfigTabDC1394::startStopCapture (bool start)
{
    if (QObject::sender() == pushButtonCaptureLeftDevice) {
        startStopCapture(source->getLeftCamera(), start);        
    } else {
        startStopCapture(source->getRightCamera(), start);
    }
}

void ConfigTabDC1394::startStopCapture (CameraDC1394 *device, bool start)
{
    if (!device) {
        return;
    }
    
    if (start) {
        device->startCapture();
    } else {
        device->stopCapture();
    }
}

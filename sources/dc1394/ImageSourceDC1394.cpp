#include "ImageSourceDC1394.h"

#include "CameraDC1394.h"
#include "CameraListModel.h"


ImageSourceDC1394::ImageSourceDC1394 (QObject *parent)
    : ImageSource(parent)
{
    fw = NULL;

    cameraListModel = new CameraListModel(this);

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
}

void ImageSourceDC1394::setRightCamera (int c)
{
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabDC1394::ConfigTabDC1394 (ImageSourceDC1394 *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    QLabel *label;
    QFrame *line, *frame;
    QPushButton *button;
    QComboBox *comboBox;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>DC1394 source</u></b>", this);
    label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label, row, 0, 1, 2);

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Rescan
    tooltip = "Rescan bus for DC1394 devices.";
    
    button = new QPushButton("Rescan");
    button->setToolTip(tooltip);
    connect(button, SIGNAL(released()), source, SLOT(scanBus()));
    layout->addWidget(button, row, 0, 1, 2);

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Frame - Camera 1
    frame = new QFrame(this);
    frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    frame->setLayout(new QVBoxLayout(frame));
    layout->addWidget(frame, row, 0, 1, 1);

    // Label
    label = new QLabel("<b>Left device</b>", this);
    label->setAlignment(Qt::AlignCenter);
    frame->layout()->addWidget(label);

    // Combo box
    tooltip = "Left DC1394 device.";
    
    comboBox = new QComboBox(this);
    comboBox->setModel(source->getCameraListModel());
    comboBox->setToolTip(tooltip);
    connect(comboBox, SIGNAL(currentChanged(int)), this, SLOT(cameraLeftSelected(int)));
    frame->layout()->addWidget(comboBox);

    // Frame - Camera 2
    frame = new QFrame(this);
    frame->setLayout(new QVBoxLayout(frame));
    frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    layout->addWidget(frame, row, 1, 1, 1);

    // Label
    label = new QLabel("<b>Right device</b>", this);
    label->setAlignment(Qt::AlignCenter);
    frame->layout()->addWidget(label);

    // Combo box
    tooltip = "Right DC1394 device.";
    
    comboBox = new QComboBox(this);
    comboBox->setModel(source->getCameraListModel());
    connect(comboBox, SIGNAL(currentChanged(int)), this, SLOT(cameraRightSelected(int)));
    frame->layout()->addWidget(comboBox);

    row++;

    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);
}

ConfigTabDC1394::~ConfigTabDC1394 ()
{
}


void ConfigTabDC1394::cameraLeftSelected (int index)
{
    QVariant c = comboBoxLeftDevice->itemData(index);
    source->setLeftCamera(c.isValid() ? c.toInt() : -1);
}

void ConfigTabDC1394::cameraRightSelected (int index)
{
    QVariant c = comboBoxRightDevice->itemData(index);
    source->setRightCamera(c.isValid() ? c.toInt() : -1);
}

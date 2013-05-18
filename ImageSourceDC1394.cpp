#include "ImageSourceDC1394.h"

ImageSourceDC1394::ImageSourceDC1394 (QObject *parent)
    : ImageSource(parent)
{
    shortName = "DC1394";
    configWidget = new ConfigTabDC1394(this);
}

ImageSourceDC1394::~ImageSourceDC1394 ()
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
    QFrame *line;

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

    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);
}

ConfigTabDC1394::~ConfigTabDC1394 ()
{
}

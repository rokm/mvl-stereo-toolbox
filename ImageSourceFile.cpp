#include "ImageSourceFile.h"

#include <opencv2/highgui/highgui.hpp>


ImageSourceFile::ImageSourceFile (QObject *parent)
    : ImageSource(parent)
{
    shortName = "FILE";
    configWidget = new ConfigTabFile(this);
}

ImageSourceFile::~ImageSourceFile ()
{
}


const QString &ImageSourceFile::getFilenameLeft () const
{
    return filenameLeft;
}

const QString &ImageSourceFile::getFilenameRight () const
{
    return filenameRight;
}


void ImageSourceFile::loadImagePair (const QString &left, const QString &right)
{
    filenameLeft = left;
    imageLeft = cv::imread(filenameLeft.toStdString());
    
    filenameRight = right;
    imageRight = cv::imread(filenameRight.toStdString());

    emit imagesChanged();
}


// *********************************************************************
// *                           Config widget                           *
// *********************************************************************
ConfigTabFile::ConfigTabFile (ImageSourceFile *s, QWidget *parent)
    : QWidget(parent), source(s)
{
    QGridLayout *layout = new QGridLayout(this);
    int row = 0;

    QLabel *label;
    QPushButton *button;
    QFrame *line;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>File source</u><b>", this);
    label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label, row, 0, 1, 2);

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Load
    tooltip = "Load new pair of images.";
    
    button = new QPushButton("Load images", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(released()), this, SLOT(loadImages()));
    layout->addWidget(button, row, 0, 1, 2);
    buttonLoadImages = button;

    row++;

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(line, row, 0, 1, 2);

    row++;

    // Left image
    tooltip = "Left image name.";

    label = new QLabel("<b>Left image: </b>", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0, 1, 2);
    labelFilenameLeft = label;

    row++;

    // Right image
    tooltip = "Right image name.";

    label = new QLabel("<b>Right image: </b>", this);
    label->setToolTip(tooltip);
    layout->addWidget(label, row, 0, 1, 2);
    labelFilenameRight = label;

    row++;

    // Spacer for padding
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addItem(spacer, row, 0, 1, 2);
}

ConfigTabFile::~ConfigTabFile ()
{
}

void ConfigTabFile::loadImages ()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Load left and right image", QString(), "Images (*.png *.jpg *.pgm *.ppm *.tif *.bmp)");

    // Take first two images
    if (filenames.size() >= 2) {
        // Use QFileInfo to get basenames
        QFileInfo fileLeft(filenames[0]);
        QFileInfo fileRight(filenames[1]);

        // Load image pair
        source->loadImagePair(filenames[0], filenames[1]);

        // Display filenames
        labelFilenameLeft->setText("<b>Left image: </b>" + fileLeft.fileName());
        labelFilenameRight->setText("<b>Right image: </b>" + fileRight.fileName());
    }
}


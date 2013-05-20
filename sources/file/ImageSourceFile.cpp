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


void ImageSourceFile::loadImagePair (const QString &left, const QString &right)
{
    filenameLeft = left;
    imageLeft = cv::imread(filenameLeft.toStdString());
    
    filenameRight = right;
    imageRight = cv::imread(filenameRight.toStdString());

    emit imagesChanged();
}


// *********************************************************************
// *                          Left image info                          *
// *********************************************************************
const QString &ImageSourceFile::getLeftFilename () const
{
    return filenameLeft;
}

int ImageSourceFile::getLeftWidth () const
{
    return imageLeft.cols;
}

int ImageSourceFile::getLeftHeight () const
{
    return imageLeft.rows;
}

int ImageSourceFile::getLeftChannels () const
{
    return imageLeft.channels();
}


// *********************************************************************
// *                         Right image info                          *
// *********************************************************************
const QString &ImageSourceFile::getRightFilename () const
{
    return filenameRight;
}

int ImageSourceFile::getRightWidth () const
{
    return imageRight.cols;
}

int ImageSourceFile::getRightHeight () const
{
    return imageRight.rows;
}

int ImageSourceFile::getRightChannels () const
{
    return imageRight.channels();
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
        // Load image pair
        source->loadImagePair(filenames[0], filenames[1]);

        // Display image information
        QFileInfo fileLeft(source->getLeftFilename());
        QFileInfo fileRight(source->getRightFilename());

        labelFilenameLeft->setText(QString("<b>Left image: </b> %1, %2x%3, %4 ch.").arg(fileLeft.fileName()).arg(source->getLeftWidth()).arg(source->getLeftHeight()).arg(source->getLeftChannels()));
        labelFilenameRight->setText(QString("<b>Right image: </b> %1, %2x%3, %4 ch.").arg(fileRight.fileName()).arg(source->getRightWidth()).arg(source->getRightHeight()).arg(source->getRightChannels()));
    }
}


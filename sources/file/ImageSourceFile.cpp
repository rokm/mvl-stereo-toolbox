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
    QFormLayout *layout = new QFormLayout(this);

    QLabel *label;
    QPushButton *button;
    QFrame *line;
    QString tooltip;

    // Name
    label = new QLabel("<b><u>File source</u><b>", this);
    label->setAlignment(Qt::AlignHCenter);

    layout->addRow(label);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Load
    tooltip = "Load new pair of images.";
    
    button = new QPushButton("Load images", this);
    button->setToolTip(tooltip);
    connect(button, SIGNAL(released()), this, SLOT(loadImages()));
    buttonLoadImages = button;

    layout->addRow(button);

    // Separator
    line = new QFrame(this);
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    layout->addRow(line);

    // Left image
    tooltip = "Left image name.";

    label = new QLabel("<b>Left image: </b>", this);
    label->setToolTip(tooltip);
    labelFilenameLeft = label;

    layout->addRow(label);

    // Right image
    tooltip = "Right image name.";

    label = new QLabel("<b>Right image: </b>", this);
    label->setToolTip(tooltip);
    labelFilenameRight = label;

    layout->addRow(label);
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

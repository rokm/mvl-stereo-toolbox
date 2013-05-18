#include "GuiImageSource.h"

#include "StereoPipeline.h"
#include "ImageSource.h"

#include <opencv2/core/core.hpp>


static inline QImage cvMatToQImage (const cv::Mat &src)
{
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);
    QRgb *destrow;
    int x, y;
    
    if (src.channels() == 3) {
        // RGB
        for (y = 0; y < src.rows; ++y) {
            const cv::Vec3b *srcrow = src.ptr<cv::Vec3b>(y);
            destrow = (QRgb*)dest.scanLine(y);
            for (x = 0; x < src.cols; ++x) {
                destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
            }
        }
    } else {
        // Gray
        for (int y = 0; y < src.rows; ++y) {
            const unsigned char *srcrow = src.ptr<unsigned char>(y);
            destrow = (QRgb*)dest.scanLine(y);
            for (int x = 0; x < src.cols; ++x) {
                destrow[x] = qRgba(srcrow[x], srcrow[x], srcrow[x], 255);
            }
        }
    }
    
    return dest;
}



GuiImageSource::GuiImageSource (StereoPipeline *p, QList<ImageSource *> &s, QWidget *parent)
    : QWidget(parent), pipeline(p), sources(s)
{
    setWindowTitle("Image source");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Left image
    labelImageLeft = new QLabel("Left image", this);
    labelImageLeft->setToolTip("Left image");
    labelImageLeft->setFrameStyle(QFrame::Box | QFrame::Sunken);
    labelImageLeft->setAlignment(Qt::AlignCenter);
    labelImageLeft->setMinimumSize(384, 288);
    layout->addWidget(labelImageLeft, 0, 0);

    // Right image
    labelImageRight = new QLabel("Right image", this);
    labelImageRight->setToolTip("Right image");
    labelImageRight->setFrameStyle(QFrame::Box | QFrame::Sunken);
    labelImageRight->setAlignment(Qt::AlignCenter);
    labelImageRight->setMinimumSize(384, 288);
    layout->addWidget(labelImageRight, 0, 1);

    // Sources tab widget
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(scrollArea, 1, 0, 1, 2);
    
    QTabWidget *tabWidget = new QTabWidget(scrollArea);
    tabWidget->setTabPosition(QTabWidget::West);

    scrollArea->setWidget(tabWidget);
    scrollArea->setWidgetResizable(true);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar, 2, 0, 1, 2);


    // Create config tabs
    for (int i = 0; i < sources.size(); i++) {
        tabWidget->addTab(sources[i]->getConfigWidget(), sources[i]->getShortName());
    }

    // Method selection
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setSource(int)));
    setSource(tabWidget->currentIndex());

    // Pipeline
    connect(pipeline, SIGNAL(inputImagesChanged()), this, SLOT(updateImages()));
}

GuiImageSource::~GuiImageSource ()
{
}

void GuiImageSource::setSource (int i)
{
    pipeline->setImageSource(sources[i]);
}


void GuiImageSource::updateImages ()
{
    labelImageLeft->setPixmap(QPixmap::fromImage(cvMatToQImage(pipeline->getLeftImage())));
    labelImageRight->setPixmap(QPixmap::fromImage(cvMatToQImage(pipeline->getRightImage())));
}

#include "GuiStereoMethod.h"

#include "StereoPipeline.h"
#include "StereoMethod.h"

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



GuiStereoMethod::GuiStereoMethod (StereoPipeline *p, QList<StereoMethod *> &m, QWidget *parent)
    : QWidget(parent), pipeline(p), methods(m)
{
    setWindowTitle("Stereo method");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Left image
    labelDisparityImage = new QLabel("Disparity image", this);
    labelDisparityImage->setFrameStyle(QFrame::Box | QFrame::Sunken);
    labelDisparityImage->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelDisparityImage, 0, 1);

    // Methods
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(scrollArea, 0, 0);
    
    QTabWidget *tabWidget = new QTabWidget(scrollArea);
    tabWidget->setTabPosition(QTabWidget::West);

    scrollArea->setWidget(tabWidget);
    scrollArea->setWidgetResizable(true);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar, 2, 0, 1, 2);

    // Create config tabs
    for (int i = 0; i < methods.size(); i++) {
        tabWidget->addTab(methods[i]->getConfigWidget(), methods[i]->getShortName());
    }

    // Method selection
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setMethod(int)));
    setMethod(tabWidget->currentIndex());

    // Pipeline
    connect(pipeline, SIGNAL(depthImageChanged()), this, SLOT(updateImage()));
}

GuiStereoMethod::~GuiStereoMethod ()
{
}

void GuiStereoMethod::setMethod (int i)
{
    pipeline->setStereoMethod(methods[i]);
}


void GuiStereoMethod::updateImage ()
{
    labelDisparityImage->setPixmap(QPixmap::fromImage(cvMatToQImage(pipeline->getDepthImage())));
    statusBar->showMessage(QString("Disparity image computed in %1 milliseconds").arg(pipeline->getDepthImageComputationTime()));
}

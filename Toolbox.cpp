#include "Toolbox.h"

#include "StereoPipeline.h"
#include "StereoMethodBlockMatching.h"
#include "StereoMethodSemiGlobalBlockMatching.h"

#include <opencv2/highgui/highgui.hpp>


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



// *********************************************************************
// *                             Toolbox                               *
// *********************************************************************
Toolbox::Toolbox (QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);

    // Stereo pipeline
    pipeline = new StereoPipeline(this);
    connect(pipeline, SIGNAL(inputImagesChanged()), this, SLOT(updateInputImages()));
    connect(pipeline, SIGNAL(depthImageChanged()), this, SLOT(updateDepthImage()));

    // Stereo Methods
    methods.append(new StereoMethodBlockMatching(this)); // OpenCV block matching
    methods.append(new StereoMethodSemiGlobalBlockMatching(this)); // OpenCV semi-global block matching

    // Create config tabs
    for (int i = 0; i < methods.size(); i++) {
        tabWidget->addTab(methods[i]->getConfigWidget(), methods[i]->getShortName());
    }
    pipeline->setStereoMethod(methods[0]);

    // Test :)
    cv::Mat imgL = cv::imread("tsukuba/scene1.row3.col3.ppm");
    cv::Mat imgR = cv::imread("tsukuba/scene1.row3.col5.ppm");
    
    pipeline->processImagePair(imgL, imgR);
}

Toolbox::~Toolbox ()
{
}


void Toolbox::updateInputImages ()
{
    labelImageLeft->setPixmap(QPixmap::fromImage(cvMatToQImage(pipeline->getLeftImage())));
    labelImageRight->setPixmap(QPixmap::fromImage(cvMatToQImage(pipeline->getRightImage())));
}


void Toolbox::updateDepthImage ()
{
    labelImageDepth->setPixmap(QPixmap::fromImage(cvMatToQImage(pipeline->getDepthImage())));
}

#include "GuiCalibration.h"

#include "ImageDisplayWidget.h"

#include "StereoPipeline.h"
#include "StereoCalibration.h"

#include <opencv2/core/core.hpp>


GuiCalibration::GuiCalibration (StereoPipeline *p, StereoCalibration *c, QWidget *parent)
    : QWidget(parent), pipeline(p), calibration(c)
{
    setWindowTitle("Stereo calibration");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Rectified image pair
    displayRectified = new ImageDisplayWidget("Rectified image pair", this);
    displayRectified->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayRectified, 0, 1);

    // Pipeline
    connect(pipeline, SIGNAL(rectifiedImagesChanged()), this, SLOT(updateImage()));
}

GuiCalibration::~GuiCalibration ()
{
}

void GuiCalibration::updateImages ()
{
    #if 0
    //const cv::Mat disparity = pipeline->getDisparityImage();

    // Show image
    displayDisparityImage->setImage(disparity);

    // If image is valid, display computation time
    if (disparity.data) {
        statusBar->showMessage(QString("Disparity image computed in %1 milliseconds").arg(pipeline->getDisparityImageComputationTime()));
    } else {
        statusBar->clearMessage();
    }
    #endif
}

#include "GuiStereoMethod.h"

#include "ImageDisplayWidget.h"

#include "StereoPipeline.h"
#include "StereoMethod.h"

#include <opencv2/core/core.hpp>


GuiStereoMethod::GuiStereoMethod (StereoPipeline *p, QList<StereoMethod *> &m, QWidget *parent)
    : QWidget(parent), pipeline(p), methods(m)
{
    setWindowTitle("Stereo method");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Disparity image
    displayDisparityImage = new ImageDisplayWidget("Disparity image", this);
    layout->addWidget(displayDisparityImage, 0, 1);

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
    connect(pipeline, SIGNAL(disparityImageChanged()), this, SLOT(updateImage()));
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
    const cv::Mat disparity = pipeline->getDisparityImage();

    // Show image
    displayDisparityImage->setImage(disparity);

    // If image is valid, display computation time
    if (disparity.data) {
        statusBar->showMessage(QString("Disparity image computed in %1 milliseconds").arg(pipeline->getDisparityImageComputationTime()));
    }
}

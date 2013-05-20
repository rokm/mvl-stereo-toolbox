#include "GuiStereoMethod.h"

#include "ImageDisplayWidget.h"

#include "StereoPipeline.h"
#include "StereoMethod.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


GuiStereoMethod::GuiStereoMethod (StereoPipeline *p, QList<StereoMethod *> &m, QWidget *parent)
    : QWidget(parent), pipeline(p), methods(m)
{
    setWindowTitle("Stereo method");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *pushButton;
    
    layout->addLayout(buttonsLayout, 0, 0, 1, 2);

    buttonsLayout->addStretch();

    pushButton = new QPushButton("Save disparity image");
    pushButton->setToolTip("Save disparity image.");
    connect(pushButton, SIGNAL(released()), this, SLOT(saveImage()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveImage = pushButton;

    buttonsLayout->addStretch();

    // Disparity image
    displayDisparityImage = new ImageDisplayWidget("Disparity image", this);
    displayDisparityImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayDisparityImage, 2, 1);

    // Methods
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(scrollArea, 2, 0);
    
    QTabWidget *tabWidget = new QTabWidget(scrollArea);
    tabWidget->setTabPosition(QTabWidget::West);

    scrollArea->setWidget(tabWidget);
    scrollArea->setWidgetResizable(true);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar, 3, 0, 1, 2);


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
        statusBar->showMessage(QString("Disparity image (%1x%2) computed in %3 milliseconds.").arg(disparity.cols).arg(disparity.rows).arg(pipeline->getDisparityImageComputationTime()));
    } else {
        statusBar->clearMessage();
    }
}



// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void GuiStereoMethod::saveImage ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save disparity image");
    if (!fileName.isNull()) {
        QFileInfo tmpFileName(fileName);

        // Extension
        QString ext = tmpFileName.completeSuffix();
        if (ext.isEmpty()) {
            fileName += ".png";
        }

        // Create filename
        try {
            cv::imwrite(fileName.toStdString(), pipeline->getDisparityImage());
        } catch (cv::Exception e) {
            qWarning() << "Failed to save image:" << QString::fromStdString(e.what());
        }
    }
}

#include "GuiImageSource.h"

#include "ImageDisplayWidget.h"

#include "StereoPipeline.h"
#include "ImageSource.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


GuiImageSource::GuiImageSource (StereoPipeline *p, QList<ImageSource *> &s, QWidget *parent)
    : QWidget(parent), pipeline(p), sources(s)
{
    setWindowTitle("Image source");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *pushButton;
    
    layout->addLayout(buttonsLayout, 0, 0, 1, 2);

    buttonsLayout->addStretch();

    pushButton = new QPushButton("Save images", this);
    pushButton->setToolTip("Save image pair.");
    connect(pushButton, SIGNAL(released()), this, SLOT(saveImages()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveImages = pushButton;

    buttonsLayout->addStretch();

    // Left image
    displayImageLeft = new ImageDisplayWidget("Left image", this);
    displayImageLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayImageLeft, 1, 0);

    // Right image
    displayImageRight = new ImageDisplayWidget("Right image", this);
    displayImageRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(displayImageRight, 1, 1);

    // Sources tab widget
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(scrollArea, 2, 0, 1, 2);
    
    QTabWidget *tabWidget = new QTabWidget(scrollArea);
    tabWidget->setTabPosition(QTabWidget::West);

    scrollArea->setWidget(tabWidget);
    scrollArea->setWidgetResizable(true);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar, 3, 0, 1, 2);


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
    displayImageLeft->setImage(pipeline->getLeftImage());
    displayImageRight->setImage(pipeline->getRightImage());
}


// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void GuiImageSource::saveImages ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save rectified images");
    if (!fileName.isNull()) {
        QFileInfo tmpFileName(fileName);

        // Extension
        QString ext = tmpFileName.completeSuffix();
        if (ext.isEmpty()) {
            ext = "jpg";
        }

        // Create filename
        QString fileNameLeft = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "L" + "." + ext;
        QString fileNameRight = tmpFileName.absolutePath() + "/" + tmpFileName.baseName() + "R" + "." + ext;

        try {
            cv::imwrite(fileNameLeft.toStdString(), pipeline->getLeftImage());
            cv::imwrite(fileNameRight.toStdString(), pipeline->getRightImage());
        } catch (cv::Exception e) {
            qWarning() << "Failed to save images:" << QString::fromStdString(e.what());
        }
    }
}


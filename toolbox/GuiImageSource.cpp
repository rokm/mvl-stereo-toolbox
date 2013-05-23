#include "GuiImageSource.h"

#include "ImageDisplayWidget.h"

#include "StereoPipeline.h"
#include "ImageSource.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


GuiImageSource::GuiImageSource (StereoPipeline *p, QList<ImageSource *> &s, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), sources(s)
{
    setWindowTitle("Image source");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *pushButton;
    
    layout->addLayout(buttonsLayout);

    buttonsLayout->addStretch();

    pushButton = new QPushButton("Save images", this);
    pushButton->setToolTip("Save image pair.");
    connect(pushButton, SIGNAL(released()), this, SLOT(saveImages()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveImages = pushButton;

    buttonsLayout->addStretch();

    // Splitter - image pair and sources selection
    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    layout->addWidget(splitter);

    // Images
    QWidget *imagesWidget = new QWidget(this);
    splitter->addWidget(imagesWidget);

    QHBoxLayout *imagesLayout = new QHBoxLayout(imagesWidget);
    imagesWidget->setLayout(imagesLayout);
    imagesWidget->resize(800, 300); // Make sure image widget has some space
    imagesWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    displayImageLeft = new ImageDisplayWidget("Left image", this);
    displayImageLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imagesLayout->addWidget(displayImageLeft);

    displayImageRight = new ImageDisplayWidget("Right image", this);
    displayImageRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imagesLayout->addWidget(displayImageRight);

    // Sources tab widget
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    splitter->addWidget(scrollArea);
    splitter->setCollapsible(1, true);
    
    QTabWidget *tabWidget = new QTabWidget(scrollArea);
    tabWidget->setTabPosition(QTabWidget::West);

    scrollArea->setWidget(tabWidget);
    scrollArea->setWidgetResizable(true);


    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);


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
    if (i < 0 || i >= sources.size()) {
        qWarning() << "Source" << i << "does not exist!";
        return;
    }
    
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
    // Make snapshot of images - because it can take a while to get
    // the filename...
    cv::Mat tmpImg1, tmpImg2;

    pipeline->getLeftImage().copyTo(tmpImg1);
    pipeline->getRightImage().copyTo(tmpImg2);
    
    // Get filename
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
            cv::imwrite(fileNameLeft.toStdString(), tmpImg1);
            cv::imwrite(fileNameRight.toStdString(), tmpImg2);
        } catch (cv::Exception e) {
            qWarning() << "Failed to save images:" << QString::fromStdString(e.what());
        }
    }
}


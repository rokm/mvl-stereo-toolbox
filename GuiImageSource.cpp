#include "GuiImageSource.h"

#include "ImageDisplayWidget.h"

#include "StereoPipeline.h"
#include "ImageSource.h"

#include <opencv2/core/core.hpp>


GuiImageSource::GuiImageSource (StereoPipeline *p, QList<ImageSource *> &s, QWidget *parent)
    : QWidget(parent), pipeline(p), sources(s)
{
    setWindowTitle("Image source");
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    setLayout(layout);

    // Left image
    displayImageLeft = new ImageDisplayWidget("Left image", this);
    layout->addWidget(displayImageLeft, 0, 0);

    // Right image
    displayImageRight = new ImageDisplayWidget("Right image", this);
    layout->addWidget(displayImageRight, 0, 1);

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
    displayImageLeft->setImage(pipeline->getLeftImage());
    displayImageRight->setImage(pipeline->getRightImage());
}

/*
 * MVL Stereo Toolbox: image pair source window
 * Copyright (C) 2013 Rok Mandeljc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "window_image_pair_source.h"
#include "image_display_widget.h"

#include <stereo_pipeline.h>
#include <image_pair_source.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


WindowImagePairSource::WindowImagePairSource (StereoPipeline *p, QList<ImagePairSource *> &s, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), sources(s)
{
    setWindowTitle("Image source");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *pushButton;
    
    layout->addLayout(buttonsLayout);

    buttonsLayout->addStretch();

    pushButton = new QPushButton("Save images", this);
    pushButton->setToolTip("Save image pair by asking for filename each time.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(saveImages()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveImages = pushButton;

    pushButton = new QPushButton("Snap images", this);
    pushButton->setToolTip("Save image pair by asking for filename only once and then appending counter number for each new snapshot.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(snapshotImages()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSnapshotImages = pushButton;

    buttonsLayout->addStretch();

    // Splitter - image pair and sources selection
    QSplitter *splitter = new QSplitter(Qt::Vertical, this);
    layout->addWidget(splitter);

    // Images
    QWidget *imagesWidget = new QWidget(this);
    splitter->addWidget(imagesWidget);

    QHBoxLayout *imagesLayout = new QHBoxLayout(imagesWidget);
    imagesLayout->setContentsMargins(0, 0, 0, 0);
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
        tabWidget->addTab(sources[i]->createConfigWidget(this), sources[i]->getShortName());
    }

    // Method selection
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setSource(int)));
    setSource(tabWidget->currentIndex());

    // Pipeline
    connect(pipeline, SIGNAL(inputImagesChanged()), this, SLOT(updateImages()));
}

WindowImagePairSource::~WindowImagePairSource ()
{
}

void WindowImagePairSource::setSource (int i)
{
    if (i < 0 || i >= sources.size()) {
        qWarning() << "Source" << i << "does not exist!";
        return;
    }
    
    pipeline->setImagePairSource(sources[i]);
}


void WindowImagePairSource::updateImages ()
{
    displayImageLeft->setImage(pipeline->getLeftImage());
    displayImageRight->setImage(pipeline->getRightImage());
}


// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void WindowImagePairSource::saveImages ()
{
    // Make snapshot of images - because it can take a while to get
    // the filename...
    cv::Mat tmpImg1, tmpImg2;

    pipeline->getLeftImage().copyTo(tmpImg1);
    pipeline->getRightImage().copyTo(tmpImg2);
    
    // Get filename
    QString fileName = QFileDialog::getSaveFileName(this, "Save rectified images");
    if (fileName.isNull()) {
        return;
    }

    // Get filename components
    QFileInfo tmpFileName(fileName);
    QDir dir = tmpFileName.absoluteDir();
    QString base = tmpFileName.baseName();
    QString ext = tmpFileName.completeSuffix();

    if (ext.isEmpty()) {
        ext = "ppm";
    }

    // Create filenames
    QString fileNameLeft = QString("%1L.%2").arg(base).arg(ext);
    QString fileNameRight = QString("%1R.%2").arg(base).arg(ext);

    try {
        cv::imwrite(dir.absoluteFilePath(fileNameLeft).toStdString(), tmpImg1);
        cv::imwrite(dir.absoluteFilePath(fileNameRight).toStdString(), tmpImg2);
    } catch (cv::Exception e) {
        qWarning() << "Failed to save images:" << QString::fromStdString(e.what());
    }
}

void WindowImagePairSource::snapshotImages ()
{
    // Make snapshot of images - because it can take a while to get
    // the filename...
    cv::Mat tmpImg1, tmpImg2;

    pipeline->getLeftRectifiedImage().copyTo(tmpImg1);
    pipeline->getRightRectifiedImage().copyTo(tmpImg2);
    
    // Get basename if not already set
    if (snapshotBaseName.isEmpty()) {
        snapshotBaseName = QFileDialog::getSaveFileName(this, "Select basename for images snapshots", "image.ppm");
        if (snapshotBaseName.isNull()) {
            return;
        }
    }

    // Get filename components
    QFileInfo tmpFileName(snapshotBaseName);
    QDir dir = tmpFileName.absoluteDir();
    QString base = tmpFileName.baseName();
    QString ext = tmpFileName.completeSuffix();

    if (ext.isEmpty()) {
        ext = "ppm";
    }
    
    // Now, construct filename, and find unoccupied counter value
    QString fileNameLeft, fileNameRight;
    for (int c = 1; ; c++) {
        fileNameLeft = QString("%1-%2L.%3").arg(base).arg(c).arg(ext);
        fileNameRight = QString("%1-%2R.%3").arg(base).arg(c).arg(ext);

        if (dir.exists(fileNameLeft) || dir.exists(fileNameRight)) {
            continue;
        }

        try {
            cv::imwrite(dir.absoluteFilePath(fileNameLeft).toStdString(), tmpImg1);
            cv::imwrite(dir.absoluteFilePath(fileNameRight).toStdString(), tmpImg2);
        } catch (cv::Exception e) {
            qWarning() << "Failed to save images:" << QString::fromStdString(e.what());
            snapshotBaseName = QString(); // Clear the image basename
        }

        break;
    }
}


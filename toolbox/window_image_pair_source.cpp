/*
 * MVL Stereo Toolbox: image pair source window
 * Copyright (C) 2013-2015 Rok Mandeljc
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

#include <stereo-pipeline/pipeline.h>
#include <stereo-pipeline/image_pair_source.h>
#include <stereo-widgets/image_display_widget.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {


WindowImagePairSource::WindowImagePairSource (Pipeline::Pipeline *p, QList<QObject *> &s, QWidget *parent)
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
    connect(pushButton, &QPushButton::clicked, this, &WindowImagePairSource::saveImages);
    buttonsLayout->addWidget(pushButton);

    pushButton = new QPushButton("Snap images", this);
    pushButton->setToolTip("Save image pair by asking for filename only once and then appending counter number for each new snapshot.");
    pushButton->setShortcut(QKeySequence(Qt::Key_F5));
    connect(pushButton, &QPushButton::clicked, this, &WindowImagePairSource::snapshotImages);
    buttonsLayout->addWidget(pushButton);

    pushButton = new QPushButton("Snapshot filename", this);
    pushButton->setToolTip("Set filename for image snapshot saving.");
    connect(pushButton, &QPushButton::clicked, this, &WindowImagePairSource::selectSnapshotFilename);
    buttonsLayout->addWidget(pushButton);

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

    displayImageLeft = new Widgets::ImageDisplayWidget("Left image", this);
    displayImageLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imagesLayout->addWidget(displayImageLeft);

    displayImageRight = new Widgets::ImageDisplayWidget("Right image", this);
    displayImageRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imagesLayout->addWidget(displayImageRight);

    // Sources tab widget
    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->setTabPosition(QTabWidget::West);
    tabWidget->setUsesScrollButtons(true);

    splitter->addWidget(tabWidget);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);


    // Create config tabs
    for (int i = 0; i < sources.size(); i++) {
        Pipeline::ImagePairSource *source = qobject_cast<Pipeline::ImagePairSource * >(sources[i]);
        tabWidget->addTab(source->createConfigWidget(this), source->getShortName());
    }

    // Method selection
    connect(tabWidget, &QTabWidget::currentChanged, this, &WindowImagePairSource::setSource);
    setSource(tabWidget->currentIndex());

    // Pipeline
    connect(pipeline, &Pipeline::Pipeline::inputImagesChanged, this, [this] () {
        cv::Mat imageLeft, imageRight;
        pipeline->getImages(imageLeft, imageRight);

        displayImageLeft->setImage(imageLeft);
        displayImageRight->setImage(imageRight);
    });

    // Pipeline's error signalization
    connect(pipeline, &Pipeline::Pipeline::error, this, [this] (int errorType, const QString &errorMessage) {
        if (errorType == Pipeline::Pipeline::ErrorImagePairSource) {
            QMessageBox::warning(this, "Image Pair Source Error", errorMessage);
        }
    });
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


// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void WindowImagePairSource::saveImages ()
{
    // Make snapshot of images - because it can take a while to get
    // the filename...
    cv::Mat imageLeft, imageRight;
    pipeline->getImages(imageLeft, imageRight);

    // Make sure images are actually available
    if (imageLeft.empty() || imageRight.empty()) {
        QMessageBox::information(this, "No data", "No data to export!");
        return;
    }

    // Get filename
    QString fileName = QFileDialog::getSaveFileName(this, "Save image pair");
    if (fileName.isNull()) {
        return;
    }

    // Get filename components
    QFileInfo tmpFileName(fileName);
    QDir dir = tmpFileName.absoluteDir();
    QString base = tmpFileName.baseName();
    QString ext = tmpFileName.completeSuffix();

    if (ext.isEmpty()) {
        ext = "png";
    }

    // Create filenames
    QString fileNameLeft = QString("%1L.%2").arg(base).arg(ext);
    QString fileNameRight = QString("%1R.%2").arg(base).arg(ext);

    try {
        cv::imwrite(dir.absoluteFilePath(fileNameLeft).toStdString(), imageLeft);
        cv::imwrite(dir.absoluteFilePath(fileNameRight).toStdString(), imageRight);
    } catch (const cv::Exception &e) {
        QMessageBox::warning(this, "Error", "Failed to save image pair: " + QString::fromStdString(e.what()));
    }
}

void WindowImagePairSource::snapshotImages ()
{
    // Make snapshot of images - because it can take a while to get
    // the filename...
    cv::Mat imageLeft, imageRight;
    pipeline->getImages(imageLeft, imageRight);

    // Make sure images are actually available
    if (imageLeft.empty() || imageRight.empty()) {
        QMessageBox::information(this, "No data", "No data to export!");
        return;
    }

    // Get basename if not already set
    if (snapshotBaseName.isEmpty()) {
        selectSnapshotFilename();
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
        ext = "png";
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
            cv::imwrite(dir.absoluteFilePath(fileNameLeft).toStdString(), imageLeft);
            cv::imwrite(dir.absoluteFilePath(fileNameRight).toStdString(), imageRight);
        } catch (const cv::Exception &e) {
            QMessageBox::warning(this, "Error", "Failed to save image pair: " + QString::fromStdString(e.what()));
            snapshotBaseName = QString(); // Clear the image basename
        }

        break;
    }
}

void WindowImagePairSource::selectSnapshotFilename ()
{
    snapshotBaseName = QFileDialog::getSaveFileName(this, "Select basename for images snapshots", snapshotBaseName.isEmpty() ? "image.png" : snapshotBaseName);
}


} // GUI
} // StereoToolbox
} // MVL

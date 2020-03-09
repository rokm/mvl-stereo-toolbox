/*
 * MVL Stereo Toolbox: image pair source window
 * Copyright (C) 2013-2017 Rok Mandeljc
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
#include <stereo-pipeline/utils.h>
#include <stereo-widgets/image_display_widget.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {


WindowImagePairSource::WindowImagePairSource (Pipeline::Pipeline *pipeline, QList<QObject *> &sources, QWidget *parent)
    : QWidget(parent, Qt::Window),
      pipeline(pipeline),
      sources(sources),
      leftInfo({ false, 0, 0, 0 }),
      rightInfo({ false, 0, 0, 0 }),
      numDroppedFrames(0),
      estimatedFps(0.0f)
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
    buttonsLayout->addWidget(pushButton, 1);

    pushButton = new QPushButton("Snap images", this);
    pushButton->setToolTip("Save image pair by asking for filename only once and then appending counter number for each new snapshot.");
    pushButton->setShortcut(QKeySequence(Qt::Key_F5));
    connect(pushButton, &QPushButton::clicked, this, &WindowImagePairSource::snapshotImages);
    buttonsLayout->addWidget(pushButton, 1);

    pushButton = new QPushButton("Snapshot filename", this);
    pushButton->setToolTip("Set filename for image snapshot saving.");
    connect(pushButton, &QPushButton::clicked, this, &WindowImagePairSource::selectSnapshotFilename);
    buttonsLayout->addWidget(pushButton, 1);

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

    statusBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(statusBar, &QStatusBar::customContextMenuRequested, this, [this] () {
        // Pop up a frame-limiter dialog
        QDialog dialog(this);
        dialog.setWindowTitle("Frame limiter");

        QVBoxLayout layout(&dialog);
        QHBoxLayout hbox;

        QLabel label("Max FPS: ");
        hbox.addWidget(&label);

        QDoubleSpinBox spinBox;
        spinBox.setDecimals(2);
        spinBox.setRange(0, 120);
        spinBox.setValue(this->pipeline->getImageCaptureFramerateLimit());
        spinBox.setToolTip("Enforce maximum framerate limit (0 = disabled)");
        hbox.addWidget(&spinBox);

        layout.addLayout(&hbox);

        QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout.addWidget(&buttons);

        if (dialog.exec() == QDialog::Accepted) {
            this->pipeline->setImageCaptureFramerateLimit(spinBox.value());
        }

        updateStatusBar();
    });

    // Create config tabs
    for (QObject *sourceObject : sources) {
        Pipeline::ImagePairSource *source = qobject_cast<Pipeline::ImagePairSource * >(sourceObject);
        tabWidget->addTab(source->createConfigWidget(this), source->getShortName());
    }

    // Method selection
    connect(tabWidget, &QTabWidget::currentChanged, this, &WindowImagePairSource::setSource);
    setSource(tabWidget->currentIndex());

    // Pipeline
    connect(pipeline, &Pipeline::Pipeline::inputImagesChanged, this, [this] () {
        cv::Mat imageLeft, imageRight;
        this->pipeline->getImages(imageLeft, imageRight);

        // Store image info for status bar
        if (!imageLeft.empty()) {
            leftInfo.valid = true;
            leftInfo.width = imageLeft.cols;
            leftInfo.height = imageLeft.rows;
            leftInfo.depth = imageLeft.depth();
        } else {
            leftInfo.valid = false;
        }
        if (!imageRight.empty()) {
            rightInfo.valid = true;
            rightInfo.width = imageRight.cols;
            rightInfo.height = imageRight.rows;
            rightInfo.depth = imageRight.depth();
        } else {
            rightInfo.valid = false;
        }

        numDroppedFrames = 0; // Reset dropped frames counter

        // Update images
        displayImageLeft->setImage(imageLeft);
        displayImageRight->setImage(imageRight);
    });

    // Pipeline's error signalization
    connect(pipeline, &Pipeline::Pipeline::error, this, [this] (int errorType, const QString &message) {
        if (errorType == Pipeline::Pipeline::ErrorImagePairSource) {
            QMessageBox::warning(this, "Image Pair Source Error", message);
        }
    });

    // Image pair source dropped frames counter
    connect(pipeline, &Pipeline::Pipeline::imageCaptureFrameDropped, this, [this] (int count) {
        numDroppedFrames = count;
        updateStatusBar();
    });
    connect(pipeline, &Pipeline::Pipeline::imageCaptureFramerateUpdated, this, [this] (float fps) {
        estimatedFps = fps;
        updateStatusBar();
    });
}

WindowImagePairSource::~WindowImagePairSource ()
{
}

void WindowImagePairSource::setSource (int idx)
{
    if (idx < 0 || idx >= sources.size()) {
        qWarning() << "Source" << idx << "does not exist!";
        return;
    }

    pipeline->setImagePairSource(sources[idx]);
}


void WindowImagePairSource::updateStatusBar ()
{
    float fpsLimit = pipeline->getImageCaptureFramerateLimit();
    if (fpsLimit != 0.0f) {
        statusBar->showMessage(QString("Image: %1x%2 %3, %4x%5 %6. FPS: %7 (limit: %8), dropped %9 frames")
            .arg(leftInfo.width)
            .arg(leftInfo.height)
            .arg(Pipeline::Utils::cvDepthToString(leftInfo.depth))
            .arg(rightInfo.width)
            .arg(rightInfo.height)
            .arg(Pipeline::Utils::cvDepthToString(rightInfo.depth))
            .arg(estimatedFps, 0, 'f' , 2)
            .arg(fpsLimit, 0, 'f' , 2)
            .arg(numDroppedFrames));
    } else {
        statusBar->showMessage(QString("Image: %1x%2 %3, %4x%5 %6. FPS: %7 (right-click to set limit), dropped %8 frames")
            .arg(leftInfo.width)
            .arg(leftInfo.height)
            .arg(Pipeline::Utils::cvDepthToString(leftInfo.depth))
            .arg(rightInfo.width)
            .arg(rightInfo.height)
            .arg(Pipeline::Utils::cvDepthToString(rightInfo.depth))
            .arg(estimatedFps, 0, 'f' , 2)
            .arg(numDroppedFrames));
    }

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
    if (imageLeft.empty() && imageRight.empty()) {
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
        if (!imageLeft.empty()) {
            cv::imwrite(dir.absoluteFilePath(fileNameLeft).toStdString(), imageLeft);
        }
        if (!imageRight.empty()) {
            cv::imwrite(dir.absoluteFilePath(fileNameRight).toStdString(), imageRight);
        }
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
    if (imageLeft.empty() && imageRight.empty()) {
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

        if ((!imageLeft.empty() && dir.exists(fileNameLeft)) || (!imageRight.empty() && dir.exists(fileNameRight))) {
            continue;
        }

        try {
            if (!imageLeft.empty()) {
                cv::imwrite(dir.absoluteFilePath(fileNameLeft).toStdString(), imageLeft);
            }
            if (!imageRight.empty()) {
                cv::imwrite(dir.absoluteFilePath(fileNameRight).toStdString(), imageRight);
            }
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

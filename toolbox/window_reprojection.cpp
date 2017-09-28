/*
 * MVL Stereo Toolbox: reprojection window
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

#include "window_reprojection.h"

#include <stereo-pipeline/pipeline.h>
#include <stereo-pipeline/reprojection.h>
#include <stereo-pipeline/utils.h>
#include <stereo-widgets/reprojection_display_widget.h>

#include <opencv2/imgcodecs.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {


WindowReprojection::WindowReprojection (Pipeline::Pipeline *pipeline, QWidget *parent)
    : QWidget(parent, Qt::Window),
      pipeline(pipeline),
      reprojection(pipeline->getReprojection()),
      pointsInfo({ false, 0, 0 }),
      numDroppedFrames(0),
      estimatedFps(0.0f)
{
    setWindowTitle("Reprojection");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    QComboBox *comboBox;
    QLabel *label;
    QHBoxLayout *box;
    QPushButton *pushButton;

    layout->addLayout(buttonsLayout);

    buttonsLayout->addStretch();

    // Display image
    box = new QHBoxLayout();
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(2);
    buttonsLayout->addLayout(box);

    label = new QLabel("Image: ", this);
    label->setToolTip("Image to display in background.");
    box->addWidget(label);

    comboBox = new QComboBox(this);
    comboBox->addItem("Disparity");
    comboBox->addItem("Left");
    comboBox->addItem("Right");
    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this] (int index) {
        switch (index) {
            case 0: {
                // Disparity visualization
                displayReprojectedImage->setImage(this->pipeline->getDisparityVisualization());
                break;
            }
            case 1: {
                // Left
                displayReprojectedImage->setImage(this->pipeline->getLeftRectifiedImage());
                break;
            }
            case 2: {
                // Right
                displayReprojectedImage->setImage(this->pipeline->getRightRectifiedImage());
                break;
            }
        }
    });
    box->addWidget(comboBox);
    comboBoxImage = comboBox;

    buttonsLayout->addStretch();

    // Reprojection method
    box = new QHBoxLayout();
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(2);
    buttonsLayout->addLayout(box);

    label = new QLabel("Reprojection method: ", this);
    label->setToolTip("Method to use for reprojection.");
    box->addWidget(label);

    comboBox = new QComboBox(this);
    box->addWidget(comboBox);
    comboBoxReprojectionMethod = comboBox;

    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this] (int index) {
        reprojection->setReprojectionMethod(comboBoxReprojectionMethod->itemData(index).toInt());
    });
    connect(reprojection, &Pipeline::Reprojection::reprojectionMethodChanged, this, [this] (int method) {
        comboBoxReprojectionMethod->blockSignals(true);
        comboBoxReprojectionMethod->setCurrentIndex(comboBoxReprojectionMethod->findData(method));
        comboBoxReprojectionMethod->blockSignals(false);
    });

    fillReprojectionMethods();
    comboBoxReprojectionMethod->setCurrentIndex(reprojection->getReprojectionMethod());

    buttonsLayout->addStretch();

    // Save reprojection data
    pushButton = new QPushButton("Save result", this);
    pushButton->setToolTip("Save reprojected points data.");
    connect(pushButton, &QPushButton::clicked, this, &WindowReprojection::saveReprojectionResult);
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveReprojection = pushButton;

    buttonsLayout->addStretch();

    // Reprojected image viewer
    displayReprojectedImage = new Widgets::ReprojectionDisplayWidget("Reprojected image", this);
    displayReprojectedImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    displayReprojectedImage->resize(400, 600); // Make sure scroll area has some size
    layout->addWidget(displayReprojectedImage);

    connect(displayReprojectedImage, &Widgets::ReprojectionDisplayWidget::coordinatesUnderMouseChanged, this, [this] (const QVector3D coordinates) {
        if (coordinates.isNull()) {
            labelCoordinates->setText("");
        } else {
            labelCoordinates->setText(QString("XYZ: %1, %2, %3").arg(coordinates.x()/1000, 0, 'f', 2).arg(coordinates.y()/1000, 0, 'f', 2).arg(coordinates.z()/1000, 0, 'f', 2));
        }
    });

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    labelCoordinates = new QLabel(statusBar);
    statusBar->addPermanentWidget(labelCoordinates);

    // Pipeline
    connect(pipeline, &Pipeline::Pipeline::visualizationChanged, this, [this] () {
        if (comboBoxImage->currentIndex() == 0) {
            // Display disparity visualization
            cv::Mat image = this->pipeline->getDisparityVisualization();
            displayReprojectedImage->setImage(image);
        }
    });

    connect(pipeline, &Pipeline::Pipeline::rectifiedImagesChanged, this, [this] () {
        if (comboBoxImage->currentIndex() == 1) {
            // Left image
            cv::Mat image = this->pipeline->getLeftRectifiedImage();
            displayReprojectedImage->setImage(image);
        } else if (comboBoxImage->currentIndex() == 2) {
            cv::Mat image = this->pipeline->getRightRectifiedImage();
            displayReprojectedImage->setImage(image);
        }
    });

    connect(pipeline, &Pipeline::Pipeline::pointsChanged, this, [this] () {
        cv::Mat points = this->pipeline->getPoints();

        displayReprojectedImage->setPoints(points);

        // If reprojected points are valid, display computation time
        if (!points.empty()) {
            pointsInfo.valid = true;
            pointsInfo.width = points.cols;
            pointsInfo.height = points.rows;
        } else {
            pointsInfo.valid = false;
        }
    });

    // Pipeline's error signalization
    connect(pipeline, &Pipeline::Pipeline::error, this, [this] (int errorType, const QString &message) {
        if (errorType == Pipeline::Pipeline::ErrorReprojection) {
            QMessageBox::warning(this, "Reprojection Error", message);
        }
    });

    // Reprojection dropped frames counter
    connect(pipeline, &Pipeline::Pipeline::reprojectionFrameDropped, this, [this] (int count) {
        numDroppedFrames = count;
        updateStatusBar();
    });
    connect(pipeline, &Pipeline::Pipeline::reprojectionFramerateUpdated, this, [this] (float fps) {
        estimatedFps = fps;
        updateStatusBar();
    });
}

WindowReprojection::~WindowReprojection ()
{
}


void WindowReprojection::updateStatusBar ()
{
    if (pointsInfo.valid) {
        statusBar->showMessage(QString("Points: %1x%2. FPS: %3, dropped %4 frames, operation time: %5 ms")
            .arg(pointsInfo.width)
            .arg(pointsInfo.height)
            .arg(estimatedFps, 0, 'f' , 2)
            .arg(numDroppedFrames)
            .arg(pipeline->getReprojectionTime())
        );
    } else {
        statusBar->showMessage("Reprojection not available.");
    }
}


// *********************************************************************
// *                        Reprojection method                        *
// *********************************************************************
void WindowReprojection::fillReprojectionMethods ()
{
    static const struct {
        int id;
        const char *text;
        const char *tooltip;
    } methods[] = {
        { Pipeline::Reprojection::MethodOpenCvCpu, "OpenCV CPU", "Stock OpenCV CPU method." },
        { Pipeline::Reprojection::MethodOpenCvCuda, "OpenCV CUDA", "Stock OpenCV CUDA method." },
    };

    const QList<int> &supportedMethods = reprojection->getSupportedReprojectionMethods();

    int item = 0;
    for (unsigned int i = 0; i < sizeof(methods)/sizeof(methods[0]); i++) {
        if (supportedMethods.contains(methods[i].id)) {
            comboBoxReprojectionMethod->addItem(methods[i].text, methods[i].id);
            comboBoxReprojectionMethod->setItemData(item++, methods[i].tooltip, Qt::ToolTipRole);
        }
    }
}


// *********************************************************************
// *                     Save reprojection result                      *
// *********************************************************************
void WindowReprojection::saveReprojectionResult ()
{
    // Make snapshot of image - because it can take a while to get
    // the filename...
    cv::Mat points = pipeline->getPoints();

    // Make sure images are actually available
    if (points.empty()) {
        QMessageBox::information(this, "No data", "No data to export!");
        return;
    }

    // Get filename
    QStringList fileFilters;
    fileFilters.append("Binary files (*.bin)");
    fileFilters.append("OpenCV storage files (*.xml *.yml *.yaml");

    QString selectedFilter = fileFilters[0];
    QString fileName = QFileDialog::getSaveFileName(this, "Save reprojected points", lastSavedFile,  fileFilters.join(";;"), &selectedFilter);
    if (!fileName.isNull()) {
        // If extension is not given, set default based on selected filter
        QString ext = QFileInfo(fileName).completeSuffix();
        if (ext.isEmpty()) {
            if (selectedFilter == fileFilters[0]) {
                ext = "bin";
            } else {
                ext = "yml";
            }
            fileName += "." + ext;
        }

        // Create file
        if (ext == "xml" || ext == "yml" || ext == "yaml") {
            // Save reprojected points in OpenCV storage format
            try {
                cv::FileStorage fs(fileName.toStdString(), cv::FileStorage::WRITE);
                fs << "points" << points;
            } catch (const cv::Exception &e) {
                QMessageBox::warning(this, "Error", "Failed to save matrix: " + QString::fromStdString(e.what()));
            }
        } else {
            // Save reprojected points in custom binary matrix format
            try {
                Utils::writeMatrixToBinaryFile(points, fileName);
            } catch (const QString &e) {
                QMessageBox::warning(this, "Error", "Failed to save binary file:" + e);
            }
        }

        lastSavedFile = fileName;
    }
}


} // GUI
} // StereoToolbox
} // MVL

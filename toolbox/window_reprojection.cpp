/*
 * MVL Stereo Toolbox: reprojection window
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

#include "window_reprojection.h"
#include "image_display_widget.h"

#include <stereo_pipeline.h>
#include <stereo_reprojection.h>

#include <opencv2/highgui/highgui.hpp>


WindowReprojection::WindowReprojection (StereoPipeline *p, StereoReprojection *r, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), reprojection(r)
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
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDisplayBackground()));
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
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(reprojectionMethodChanged(int)));
    connect(reprojection, SIGNAL(reprojectionMethodChanged(int)), this, SLOT(updateReprojectionMethod(int)));
    box->addWidget(comboBox);
    comboBoxReprojectionMethod = comboBox;

    fillReprojectionMethods();
    comboBoxReprojectionMethod->setCurrentIndex(reprojection->getReprojectionMethod());

    buttonsLayout->addStretch();

    // Save reprojection data
    pushButton = new QPushButton("Save result", this);
    pushButton->setToolTip("Save reprojected points data.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(saveReprojectionResult()));
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveReprojection = pushButton;

    buttonsLayout->addStretch();

    // Reprojected image viewer
    displayReprojectedImage = new ReprojectedImageDisplayWidget("Reprojected image", this);
    displayReprojectedImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    displayReprojectedImage->resize(400, 600); // Make sure scroll area has some size
    layout->addWidget(displayReprojectedImage);

    connect(displayReprojectedImage, SIGNAL(coordinatesUnderMouseChanged(QVector3D)), this, SLOT(displayCoordinates(const QVector3D &)));

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    labelCoordinates = new QLabel(statusBar);
    statusBar->addPermanentWidget(labelCoordinates);

    // Pipeline
    connect(pipeline, SIGNAL(disparityVisualizationImageChanged()), this, SLOT(updateDisplayBackground()));
    connect(pipeline, SIGNAL(reprojectedImageChanged()), this, SLOT(updateDisplayValues()));
}

WindowReprojection::~WindowReprojection ()
{
}


void WindowReprojection::updateDisplayBackground ()
{
    switch (comboBoxImage->currentIndex()) {
        case 0: {
            // Disparity visualization
            displayReprojectedImage->setImage(pipeline->getDisparityVisualizationImage());
            break;
        }
        case 1: {
            // Left
            displayReprojectedImage->setImage(pipeline->getLeftRectifiedImage());
            break;
        }
        case 2: {
            // Right
            displayReprojectedImage->setImage(pipeline->getRightRectifiedImage());
            break;
        }
    }
}

void WindowReprojection::updateDisplayValues ()
{
    const cv::Mat &reprojectedPoints = pipeline->getReprojectedImage();

    displayReprojectedImage->setPoints(reprojectedPoints);

    // If reprojected points are valid, display computation time
    if (!reprojectedPoints.empty()) {
        statusBar->showMessage(QString("Disparity image (%1x%2) reprojected in %3 milliseconds.").arg(reprojectedPoints.cols).arg(reprojectedPoints.rows).arg(pipeline->getReprojectionComputationTime()));
    } else {
        statusBar->showMessage("Reprojection not available.");
    }
}


void WindowReprojection::displayCoordinates (const QVector3D &coordinates)
{
    if (coordinates.isNull()) {
        labelCoordinates->setText("");
    } else {
        labelCoordinates->setText(QString("XYZ: %1, %2, %3").arg(coordinates.x()/1000, 0, 'f', 2).arg(coordinates.y()/1000, 0, 'f', 2).arg(coordinates.z()/1000, 0, 'f', 2));
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
        { StereoReprojection::ReprojectionMethodToolboxCpu, "Toolbox CPU", "Toolbox-modified CPU method (handles ROI)." },
        { StereoReprojection::ReprojectionMethodToolboxGpu, "Toolbox GPU", "Toolbox-modified GPU method (handles ROI)." },
        { StereoReprojection::ReprojectionMethodOpenCvCpu, "OpenCV CPU", "Stock OpenCV CPU method." },
        { StereoReprojection::ReprojectionMethodOpenCvGpu, "OpenCV GPU", "Stock OpenCV GPU method." },
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

void WindowReprojection::reprojectionMethodChanged (int index)
{
    reprojection->setReprojectionMethod(comboBoxReprojectionMethod->itemData(index).toInt());
}

void WindowReprojection::updateReprojectionMethod (int method)
{
    bool oldState = comboBoxReprojectionMethod->blockSignals(true);
    comboBoxReprojectionMethod->setCurrentIndex(comboBoxReprojectionMethod->findData(method));
    comboBoxReprojectionMethod->blockSignals(oldState);
}


// *********************************************************************
// *                     Save reprojection result                      *
// *********************************************************************
void WindowReprojection::saveReprojectionResult ()
{
    // Make snapshot of image - because it can take a while to get
    // the filename...
    cv::Mat tmpReprojection;

    pipeline->getReprojectedImage().copyTo(tmpReprojection);

    // Get filename
    QStringList fileFilters;
    fileFilters.append("Binary files (*.bin)");
    fileFilters.append("OpenCV storage files (*.xml *.yml *.yaml");

    QString selectedFilter = fileFilters[0];
    QString fileName = QFileDialog::getSaveFileName(this, "Save reprojection result", QString(),  fileFilters.join(";;"), &selectedFilter);
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
                fs << "points" << tmpReprojection;
            } catch (cv::Exception e) {
                qWarning() << "Failed to save matrix:" << QString::fromStdString(e.what());
            }
        } else {
            // Save reprojected points in custom binary matrix format
            try {
                StereoPipeline::writeMatrixToBinaryFile(tmpReprojection, fileName);
            } catch (QString e) {
                qWarning() << "Failed to save binary file:" << e;
            }
        }
    }
}

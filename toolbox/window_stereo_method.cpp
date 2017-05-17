/*
 * MVL Stereo Toolbox: stereo method window
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

#include "window_stereo_method.h"

#include <stereo-pipeline/pipeline.h>
#include <stereo-pipeline/stereo_method.h>
#include <stereo-pipeline/disparity_visualization.h>
#include <stereo-pipeline/utils.h>
#include <stereo-widgets/disparity_display_widget.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {


WindowStereoMethod::WindowStereoMethod (Pipeline::Pipeline *p, QList<QObject *> &m, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), methods(m), visualization(pipeline->getVisualization())
{
    setWindowTitle("Stereo method");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    QPushButton *pushButton;
    QComboBox *comboBox;
    QLabel *label;
    QHBoxLayout *box;

    layout->addLayout(buttonsLayout);

    buttonsLayout->addStretch();

    // Export parameters
    pushButton = new QPushButton("Export param.", this);
    pushButton->setToolTip("Export active method's parameters.");
    connect(pushButton, &QPushButton::clicked, this, &WindowStereoMethod::exportParameters);
    buttonsLayout->addWidget(pushButton);
    pushButtonExportParameters = pushButton;

    // Import parameters
    pushButton = new QPushButton("Import param.", this);
    pushButton->setToolTip("Import parameters for active method.");
    connect(pushButton, &QPushButton::clicked, this, &WindowStereoMethod::importParameters);
    buttonsLayout->addWidget(pushButton);
    pushButtonImportParameters = pushButton;

    // Save
    pushButton = new QPushButton("Save disparity", this);
    pushButton->setToolTip("Save disparity image.");
    connect(pushButton, &QPushButton::clicked, this, &WindowStereoMethod::saveImage);
    buttonsLayout->addWidget(pushButton);
    pushButtonSaveImage = pushButton;

    buttonsLayout->addStretch();

    // Disparity image visualization type
    box = new QHBoxLayout();
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(2);
    buttonsLayout->addLayout(box);

    label = new QLabel("Vizualization type: ", this);
    label->setToolTip("Disparity image vizualization type");
    box->addWidget(label);

    comboBox = new QComboBox(this); // Filled in by disparity image display widget!
    box->addWidget(comboBox);
    comboBoxVisualizationMethod = comboBox;

    connect(comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this] (int index) {
        visualization->setVisualizationMethod(comboBoxVisualizationMethod->itemData(index).toInt());
    });
    connect(visualization, &Pipeline::DisparityVisualization::visualizationMethodChanged, this, [this] (int method) {
        comboBoxVisualizationMethod->blockSignals(true);
        comboBoxVisualizationMethod->setCurrentIndex(comboBoxVisualizationMethod->findData(method));
        comboBoxVisualizationMethod->blockSignals(false);
    });

    fillVisualizationMethods();
    visualization->setVisualizationMethod(Pipeline::DisparityVisualization::MethodGrayscale); // Set grayscale as default

    buttonsLayout->addStretch();

    // Splitter - disparity image and methods
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(splitter);

    // Methods
    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->setUsesScrollButtons(true);
    tabWidget->setTabPosition(QTabWidget::West);

    splitter->addWidget(tabWidget);

    // Disparity image
    displayDisparityImage = new Widgets::DisparityDisplayWidget("Disparity image", this);
    displayDisparityImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    displayDisparityImage->resize(400, 600); // Make sure scroll area has some size
    splitter->addWidget(displayDisparityImage);

    connect(displayDisparityImage, &Widgets::DisparityDisplayWidget::disparityUnderMouseChanged, this, &WindowStereoMethod::displayDisparity);

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    labelDisparity = new QLabel(statusBar);
    statusBar->addPermanentWidget(labelDisparity);

    // Create config tabs
    for (int i = 0; i < methods.size(); i++) {
        Pipeline::StereoMethod *method = qobject_cast<Pipeline::StereoMethod * >(methods[i]);
        tabWidget->addTab(method->createConfigWidget(this), method->getShortName());
    }

    // Method selection
    connect(tabWidget, &QTabWidget::currentChanged, this, &WindowStereoMethod::setMethod);
    setMethod(tabWidget->currentIndex());

    // Pipeline
    // Use disparityVisualizationImageChanged() instead of disparityImageChanged()
    // to capture changes due to visualization method switch
    connect(pipeline, &Pipeline::Pipeline::disparityVisualizationChanged, this, &WindowStereoMethod::updateDisplayBackground);
    connect(pipeline, &Pipeline::Pipeline::disparityChanged, this, &WindowStereoMethod::updateDisplayValues);

    // Pipeline's error signalization
    connect(pipeline, &Pipeline::Pipeline::error, this, [this] (int errorType, const QString &errorMessage) {
        if (errorType == Pipeline::Pipeline::ErrorStereoMethod) {
            QMessageBox::warning(this, "Stereo Method Error", errorMessage);
        } else if (errorType == Pipeline::Pipeline::ErrorVisualization) {
            QMessageBox::warning(this, "Visualization Error", errorMessage);
        }
    });
}

WindowStereoMethod::~WindowStereoMethod ()
{
}

void WindowStereoMethod::setMethod (int i)
{
    if (i < 0 || i >= methods.size()) {
        qWarning() << "Method" << i << "does not exist!";
        return;
    }

    pipeline->setStereoMethod(methods[i]);
}


void WindowStereoMethod::updateDisplayBackground ()
{
    const cv::Mat &disparityVisualization = pipeline->getDisparityVisualization();

    // Disparity image
    displayDisparityImage->setImage(disparityVisualization);
}

void WindowStereoMethod::updateDisplayValues ()
{
    const cv::Mat &disparity = pipeline->getDisparity();

    // Disparity image
    displayDisparityImage->setDisparity(disparity);

    // If image is valid, display computation time
    if (!disparity.empty()) {
        statusBar->showMessage(QString("Disparity image (%1x%2, %3) computed in %4 milliseconds; dropped %5 frames.").arg(disparity.cols).arg(disparity.rows).arg(Utils::cvDepthToString(disparity.depth())).arg(pipeline->getDisparityComputationTime()).arg(pipeline->getStereoDroppedFrames()));
    } else {
        statusBar->showMessage(QString("Disparity image not available."));
    }
}

void WindowStereoMethod::displayDisparity (float disparity)
{
    if (std::isnan(disparity)) {
        labelDisparity->setText("");
    } else {
        labelDisparity->setText(QString("Disp: %1").arg(disparity, 0, 'f', 2));
    }
}


// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void WindowStereoMethod::saveImage ()
{
    // Make snapshot of image - because it can take a while to get
    // the filename...
    cv::Mat tmpDisparity;
    cv::Mat tmpDisparityVisualization;

    pipeline->getDisparity().copyTo(tmpDisparity);
    pipeline->getDisparityVisualization().copyTo(tmpDisparityVisualization);

    if (!tmpDisparity.data) {
        QMessageBox::information(this, "No data", "No data to export!");
        return;
    }

    // Get filename
    QStringList fileFilters;
    fileFilters.append("Image files (*.png *.jpg *.pgm *.ppm *.tif *.bmp)");
    fileFilters.append("Binary files (*.bin)");
    fileFilters.append("OpenCV storage files (*.xml *.yml *.yaml");

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, "Save disparity", lastSavedFile,  fileFilters.join(";;"), &selectedFilter);
    if (!fileName.isNull()) {
        // If extension is not given, set default based on selected filter
        QString ext = QFileInfo(fileName).completeSuffix();
        if (ext.isEmpty()) {
            if (selectedFilter == fileFilters[0]) {
                ext = "png";
            } else if (selectedFilter == fileFilters[1]) {
                ext = "bin";
            } else {
                ext = "yml";
            }
            fileName += "." + ext;
        }

        // Create file
        if (ext == "xml" || ext == "yml" || ext == "yaml") {
            // Save raw disparity in OpenCV storage format
            try {
                cv::FileStorage fs(fileName.toStdString(), cv::FileStorage::WRITE);
                fs << "disparity" << tmpDisparity;
            } catch (const cv::Exception &e) {
                QMessageBox::warning(this, "Error", "Failed to save matrix: " + QString::fromStdString(e.what()));
            }
        } else if (ext == "bin") {
            // Save raw disparity in custom binary matrix format
            try {
                Utils::writeMatrixToBinaryFile(tmpDisparity, fileName);
            } catch (const QString &e) {
                QMessageBox::warning(this, "Error", "Failed to save binary file: " + e);
            }
        } else {
            // Save disparity visualization as image using cv::imwrite
            if (!tmpDisparityVisualization.data) {
                QMessageBox::information(this, "No data", "No data to export!");
                return;
            }

            try {
                cv::imwrite(fileName.toStdString(), tmpDisparityVisualization);
            } catch (const cv::Exception &e) {
                QMessageBox::warning(this, "Error", "Failed to save image: " + QString::fromStdString(e.what()));
            }
        }

        lastSavedFile = fileName;
    }
}


// *********************************************************************
// *                      Parameter import/export                      *
// *********************************************************************
void WindowStereoMethod::importParameters ()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load parameters from file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        try {
            pipeline->loadStereoMethodParameters(fileName);
        } catch (QString &e) {
            QMessageBox::warning(this, "Error", "Failed to import parameters: " + e);
        }
    }
}

void WindowStereoMethod::exportParameters ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save parameters to file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        try {
            pipeline->saveStereoMethodParameters(fileName);
        } catch (QString &e) {
            QMessageBox::warning(this, "Error", "Failed to export parameters: " + e);
        }
    }
}


// *********************************************************************
// *                       Visualization method                        *
// *********************************************************************
void WindowStereoMethod::fillVisualizationMethods ()
{
    static const struct {
        int id;
        const char *text;
        const char *tooltip;
    } methods[] = {
        { Pipeline::DisparityVisualization::MethodGrayscale, "Grayscale", "Grayscale." },
        { Pipeline::DisparityVisualization::MethodColorCuda, "Color (CUDA)", "HSV color (CUDA)." },
        { Pipeline::DisparityVisualization::MethodColorCpu, "Color (CPU)", "HSV color (CPU)." },
    };

    const QList<int> &supportedMethods = visualization->getSupportedVisualizationMethods();

    int item = 0;
    for (unsigned int i = 0; i < sizeof(methods)/sizeof(methods[0]); i++) {
        if (supportedMethods.contains(methods[i].id)) {
            comboBoxVisualizationMethod->addItem(methods[i].text, methods[i].id);
            comboBoxVisualizationMethod->setItemData(item++, methods[i].tooltip, Qt::ToolTipRole);
        }
    }
}


} // GUI
} // StereoToolbox
} // MVL

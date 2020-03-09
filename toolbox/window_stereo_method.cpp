/*
 * MVL Stereo Toolbox: stereo method window
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


WindowStereoMethod::WindowStereoMethod (Pipeline::Pipeline *pipeline, QList<QObject *> &methods, QWidget *parent)
    : QWidget(parent, Qt::Window),
      pipeline(pipeline),
      methods(methods),
      visualization(pipeline->getVisualization()),
      disparityInfo({ false, 0, 0, 0 }),
      numDroppedFrames(0),
      estimatedFps(0.0f)
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

    connect(displayDisparityImage, &Widgets::DisparityDisplayWidget::disparityUnderMouseChanged, this, [this] (float disparity) {
        if (std::isnan(disparity)) {
            labelDisparity->setText("");
        } else {
            labelDisparity->setText(QString("Disp: %1").arg(disparity, 0, 'f', 2));
        }
    });

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    labelDisparity = new QLabel(statusBar);
    statusBar->addPermanentWidget(labelDisparity);

    // Create config tabs
    int defaultMethodIdx = 0;
    for (int i = 0; i < methods.size(); i++) {
        Pipeline::StereoMethod *method = qobject_cast<Pipeline::StereoMethod * >(methods[i]);
        tabWidget->addTab(method->createConfigWidget(this), method->getShortName());

        // Set BM as default method
        if (method->getShortName() == "BM") {
            defaultMethodIdx = i;
        }
    }
    tabWidget->setCurrentIndex(defaultMethodIdx);


    // Method selection
    connect(tabWidget, &QTabWidget::currentChanged, this, &WindowStereoMethod::setMethod);
    setMethod(tabWidget->currentIndex());

    // Pipeline
    connect(pipeline, &Pipeline::Pipeline::visualizationChanged, this, [this] () {
        cv::Mat image = this->pipeline->getDisparityVisualization();
        displayDisparityImage->setImage(image);
    });

    connect(pipeline, &Pipeline::Pipeline::disparityChanged, this, [this] () {
        cv::Mat disparity = this->pipeline->getDisparity();

        // Disparity
        displayDisparityImage->setDisparity(disparity);

        // If image is valid, update info for status bar
        if (!disparity.empty()) {
            disparityInfo.valid = true;
            disparityInfo.width = disparity.cols;
            disparityInfo.height = disparity.rows;
            disparityInfo.depth = disparity.depth();
        } else {
            statusBar->showMessage(QString("Disparity not available."));
        }
        numDroppedFrames = 0; // Reset dropped frames counter

        // Update status bar
        updateStatusBar();
    });

    // Pipeline's error signalization
    connect(pipeline, &Pipeline::Pipeline::error, this, [this] (int errorType, const QString &message) {
        if (errorType == Pipeline::Pipeline::ErrorStereoMethod) {
            QMessageBox::warning(this, "Stereo Method Error", message);
        } else if (errorType == Pipeline::Pipeline::ErrorVisualization) {
            QMessageBox::warning(this, "Visualization Error", message);
        }
    });

    // Stereo method dropped frames counter
    connect(pipeline, &Pipeline::Pipeline::stereoMethodFrameDropped, this, [this] (int count) {
        numDroppedFrames = count;
        updateStatusBar();
    });
    connect(pipeline, &Pipeline::Pipeline::stereoMethodFramerateUpdated, this, [this] (float fps) {
        estimatedFps = fps;
        updateStatusBar();
    });
}

WindowStereoMethod::~WindowStereoMethod ()
{
}

void WindowStereoMethod::setMethod (int idx)
{
    if (idx < 0 || idx >= methods.size()) {
        qWarning() << "Method" << idx << "does not exist!";
        return;
    }

    pipeline->setStereoMethod(methods[idx]);
}


void WindowStereoMethod::updateStatusBar ()
{
    if (disparityInfo.valid) {
        statusBar->showMessage(QString("Disparity: %1x%2, %3. FPS: %4, dropped %5 frames, operation time: %6 ms.")
            .arg(disparityInfo.width)
            .arg(disparityInfo.height)
            .arg(Pipeline::Utils::cvDepthToString(disparityInfo.depth))
            .arg(estimatedFps, 0, 'f' , 2)
            .arg(numDroppedFrames)
            .arg(pipeline->getStereoMethodTime())
        );
    } else {
        statusBar->showMessage(QString("Disparity not available."));
    }
}



// *********************************************************************
// *                            Image saving                           *
// *********************************************************************
void WindowStereoMethod::saveImage ()
{
    // Make snapshot of image - because it can take a while to get
    // the filename...
    cv::Mat disparity, visualization;

    disparity = pipeline->getDisparity();
    visualization = pipeline->getDisparityVisualization();

    if (disparity.empty()) {
        QMessageBox::information(this, "No data", "No data to export!");
        return;
    }

    // Get filename
    QStringList fileFilters;
    fileFilters.append("Image files (*.png *.jpg *.pgm *.ppm *.tif *.bmp)");
    fileFilters.append("Binary files (*.bin)");
    fileFilters.append("OpenCV storage files (*.xml *.yml *.yaml *.xml.gz *.yml.gz *.yaml.gz)");

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
                ext = "yml.gz";
            }
            fileName += "." + ext;
        }

        // Create file
        if (ext == "xml" || ext == "yml" || ext == "yaml" || ext == "xml.gz" || ext == "yml.gz" || ext == "yaml.gz") {
            // Save raw disparity in OpenCV storage format
            try {
                cv::FileStorage fs(fileName.toStdString(), cv::FileStorage::WRITE);
                fs << "disparity" << disparity;
            } catch (const std::exception &e) {
                QMessageBox::warning(this, "Error", QStringLiteral("Failed to save matrix: %1").arg(QString::fromStdString(e.what())));
            }
        } else if (ext == "bin") {
            // Save raw disparity in custom binary matrix format
            try {
                Pipeline::Utils::writeMatrixToBinaryFile(disparity, fileName);
            } catch (const std::exception &e) {
                QMessageBox::warning(this, "Error", QStringLiteral("Failed to save binary file: %1").arg(QString::fromStdString(e.what())));
            }
        } else {
            // Save disparity visualization as image using cv::imwrite
            if (visualization.empty()) {
                QMessageBox::information(this, "No data", "No data to export!");
                return;
            }

            try {
                cv::imwrite(fileName.toStdString(), visualization);
            } catch (const std::exception &e) {
                QMessageBox::warning(this, "Error", QStringLiteral("Failed to save image: %1").arg(QString::fromStdString(e.what())));
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
    QString fileName = QFileDialog::getOpenFileName(this, "Load parameters from file", QString(), "OpenCV storage files (*.xml *.yml *.yaml *.xml.gz *.yml.gz *.yaml.gz)");
    if (!fileName.isNull()) {
        try {
            pipeline->loadStereoMethodParameters(fileName);
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "Error", QStringLiteral("Failed to import parameters: %1").arg(e.what()));
        }
    }
}

void WindowStereoMethod::exportParameters ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save parameters to file", QString(), "OpenCV storage files (*.xml *.yml *.yaml *.xml.gz *.yml.gz *.yaml.gz)");
    if (!fileName.isNull()) {
        try {
            pipeline->saveStereoMethodParameters(fileName);
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "Error", QStringLiteral("Failed to export parameters: %1").arg(QString::fromStdString(e.what())));
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

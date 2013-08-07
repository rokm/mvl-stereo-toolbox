/*
 * MVL Stereo Toolbox: stereo method window
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

#include "window_stereo_method.h"
#include "image_display_widget.h"

#include <stereo_pipeline.h>
#include <stereo_method.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// Work-around for MSVC
#ifdef _MSC_VER
#define isnan(x) _isnan(x)
#endif


WindowStereoMethod::WindowStereoMethod (StereoPipeline *p, QList<StereoMethod *> &m, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p), methods(m)
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
    connect(pushButton, SIGNAL(clicked()), this, SLOT(exportParameters()));
    buttonsLayout->addWidget(pushButton);
    pushButtonExportParameters = pushButton;

    // Import parameters
    pushButton = new QPushButton("Import param.", this);
    pushButton->setToolTip("Import parameters for active method.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(importParameters()));
    buttonsLayout->addWidget(pushButton);
    pushButtonImportParameters = pushButton;

    // Save
    pushButton = new QPushButton("Save disparity", this);
    pushButton->setToolTip("Save disparity image.");
    connect(pushButton, SIGNAL(clicked()), this, SLOT(saveImage()));
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
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(visualizationMethodChanged(int)));
    connect(pipeline, SIGNAL(disparityVisualizationMethodChanged(int)), this, SLOT(updateVisualizationMethod(int)));
    box->addWidget(comboBox);
    comboBoxVisualizationMethod = comboBox;

    fillVisualizationMethods();
    pipeline->setDisparityVisualizationMethod(StereoPipeline::DisparityVisualizationGrayscale); // Set grayscale as default

    buttonsLayout->addStretch();

    // Splitter - disparity image and methods
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(splitter);
    
    // Methods
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->resize(400, 600); // Make sure scroll area has some size
    splitter->addWidget(scrollArea);
    
    QTabWidget *tabWidget = new QTabWidget(scrollArea);
    tabWidget->setTabPosition(QTabWidget::West);

    scrollArea->setWidget(tabWidget);
    scrollArea->setWidgetResizable(true);
    
    // Disparity image
    displayDisparityImage = new DisparityImageDisplayWidget("Disparity image", this);
    displayDisparityImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    displayDisparityImage->resize(400, 600); // Make sure scroll area has some size
    splitter->addWidget(displayDisparityImage);

    connect(displayDisparityImage, SIGNAL(disparityUnderMouseChanged(float)), this, SLOT(displayDisparity(float)));

    // Status bar
    statusBar = new QStatusBar(this);
    layout->addWidget(statusBar);

    labelDisparity = new QLabel(statusBar);
    statusBar->addPermanentWidget(labelDisparity);

    // Create config tabs
    for (int i = 0; i < methods.size(); i++) {
        tabWidget->addTab(methods[i]->createConfigWidget(this), methods[i]->getShortName());
    }

    // Method selection
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(setMethod(int)));
    setMethod(tabWidget->currentIndex());

    // Pipeline
    // Use disparityVisualizationImageChanged() instead of disparityImageChanged()
    // to capture changes due to visualization method switch
    connect(pipeline, SIGNAL(disparityVisualizationImageChanged()), this, SLOT(updateDisplayBackground()));
    connect(pipeline, SIGNAL(disparityImageChanged()), this, SLOT(updateDisplayValues()));
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
    const cv::Mat &disparityVisualization = pipeline->getDisparityVisualizationImage();

    // Disparity image
    displayDisparityImage->setImage(disparityVisualization);
}

void WindowStereoMethod::updateDisplayValues ()
{
    const cv::Mat &disparity = pipeline->getDisparityImage();

    // Disparity image
    displayDisparityImage->setDisparity(disparity);
    
    // If image is valid, display computation time
    if (!disparity.empty()) {
        statusBar->showMessage(QString("Disparity image (%1x%2) computed in %3 milliseconds; dropped %4 frames.").arg(disparity.cols).arg(disparity.rows).arg(pipeline->getDisparityImageComputationTime()).arg(pipeline->getStereoDroppedFrames()));
    } else {
        statusBar->showMessage(QString("Disparity image not available."));
    }
}

void WindowStereoMethod::displayDisparity (float disparity)
{
    if (isnan(disparity)) {
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
    cv::Mat tmpImg;

    pipeline->getDisparityImage().copyTo(tmpImg);

    // Get filename
    QString fileName = QFileDialog::getSaveFileName(this, "Save disparity image");
    if (!fileName.isNull()) {
        QFileInfo tmpFileName(fileName);

        // Extension
        QString ext = tmpFileName.completeSuffix();
        if (ext.isEmpty()) {
            fileName += ".png";
        }

        // Create filename
        try {
            cv::imwrite(fileName.toStdString(), tmpImg);
        } catch (cv::Exception e) {
            qWarning() << "Failed to save image:" << QString::fromStdString(e.what());
        }
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
            pipeline->getStereoMethod()->loadParameters(fileName);
        } catch (QString e) {
            QMessageBox::warning(this, "Error", "Failed to import parameters: " + e);
        }
    }
}

void WindowStereoMethod::exportParameters ()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save parameters to file", QString(), "OpenCV storage file (*.xml *.yml *.yaml)");
    if (!fileName.isNull()) {
        try {
            pipeline->getStereoMethod()->saveParameters(fileName);
        } catch (QString e) {
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
        { StereoPipeline::DisparityVisualizationNone, "None", "No visualization." },
        { StereoPipeline::DisparityVisualizationGrayscale, "Grayscale", "Grayscale." },
        { StereoPipeline::DisparityVisualizationColorGpu, "Color (GPU)", "HSV color (GPU)." },
    };
    
    const QList<int> &supportedMethods = pipeline->getSupportedDisparityVisualizationMethods();

    int item = 0;
    for (unsigned int i = 0; i < sizeof(methods)/sizeof(methods[0]); i++) {
        if (supportedMethods.contains(methods[i].id)) {
            comboBoxVisualizationMethod->addItem(methods[i].text, methods[i].id);
            comboBoxVisualizationMethod->setItemData(item++, methods[i].tooltip, Qt::ToolTipRole);
        }
    }
}

void WindowStereoMethod::visualizationMethodChanged (int index)
{
    pipeline->setDisparityVisualizationMethod(comboBoxVisualizationMethod->itemData(index).toInt());
}

void WindowStereoMethod::updateVisualizationMethod (int method)
{
    bool oldState = comboBoxVisualizationMethod->blockSignals(true);
    comboBoxVisualizationMethod->setCurrentIndex(comboBoxVisualizationMethod->findData(method));
    comboBoxVisualizationMethod->blockSignals(oldState);
}

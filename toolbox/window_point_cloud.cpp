/*
 * MVL Stereo Toolbox: point cloud window
 * Copyright (C) 2015 Rok Mandeljc
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

#include "window_point_cloud.h"

#include <stereo-pipeline/pipeline.h>
#include <stereo-pipeline/utils.h>
#include <stereo-widgets/point_cloud_visualization_widget.h>

#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {


WindowPointCloud::WindowPointCloud (Pipeline::Pipeline *p, QWidget *parent)
    : QWidget(parent, Qt::Window), pipeline(p)
{
    setWindowTitle("Point cloud");
    resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    layout->addLayout(buttonsLayout);

    // Save reprojection data
    pushButtonSavePointCloud = new QPushButton("Save point cloud", this);
    pushButtonSavePointCloud->setToolTip("Save point cloud.");
    connect(pushButtonSavePointCloud, &QPushButton::clicked, this, &WindowPointCloud::savePointCloud);
    buttonsLayout->addWidget(pushButtonSavePointCloud);

    buttonsLayout->addStretch();

    // Point cloud visualization widget
    visualizationWidget = new Widgets::PointCloudVisualizationWidget(this);
    layout->addWidget(visualizationWidget);

    // In case of a continuous video stream, this assumes that the
    // framerate is high enough that receiving a newer image before
    // the point cloud is computed does not cause noticeable artifacts...
    connect(pipeline, &Pipeline::Pipeline::rectifiedImagesChanged, this, [this] (const cv::Mat imageLeft, const cv::Mat imageRight) {
        Q_UNUSED(imageRight);
        visualizationWidget->setImage(imageLeft);
    });

    connect(pipeline, &Pipeline::Pipeline::pointsChanged, this, [this] (const cv::Mat points) {
        visualizationWidget->setPoints(points);
    });
}

WindowPointCloud::~WindowPointCloud ()
{
}


void WindowPointCloud::savePointCloud ()
{
    // Create a snapshot of current point cloud
    cv::Mat points = pipeline->getPoints();
    cv::Mat image = pipeline->getLeftRectifiedImage();

    // Make sure images are actually available
    if (points.empty() || image.empty()) {
        QMessageBox::information(this, "No data", "No data to export!");
        return;
    }

    // Get filename
    QStringList fileFilters;
    fileFilters.append("Binary PCD file (*.pcd)");
    fileFilters.append("ASCII PCD file (*.pcd)");

    QString selectedFilter = fileFilters[0];
    QString fileName = QFileDialog::getSaveFileName(this, "Save point cloud", lastSavedFile,  fileFilters.join(";;"), &selectedFilter);
    if (!fileName.isNull()) {
        // If extension is not given, set default based on selected filter
        QString ext = QFileInfo(fileName).completeSuffix();
        if (ext.isEmpty()) {
            fileName += ".pcd";
        }

        try {
            Utils::writePointCloudToPcdFile(image, points, fileName, selectedFilter == fileFilters[0]);
        } catch (const QString &e) {
            QMessageBox::warning(this, "Error", "Failed to save point cloud: " + e);
        }

        lastSavedFile = fileName;
    }
}


} // GUI
} // StereoToolbox
} // MVL

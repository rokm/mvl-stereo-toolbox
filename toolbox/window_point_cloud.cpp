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
#include "point_cloud_visualization_widget.h"

#include <stereo-pipeline/pipeline.h>

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

    // Point cloud visualization widget
    visualizationWidget = new PointCloudVisualizationWidget(this);
    layout->addWidget(visualizationWidget);

    connect(pipeline, &Pipeline::Pipeline::reprojectedImageChanged, this, [this] () {
        const cv::Mat &points = pipeline->getReprojectedImage();
        const cv::Mat &image = pipeline->getLeftRectifiedImage();
        visualizationWidget->setPointCloud(image, points);
    });
}

WindowPointCloud::~WindowPointCloud ()
{
}


} // GUI
} // StereoToolbox
} // MVL

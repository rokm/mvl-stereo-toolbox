/*
 * MVL Stereo Toolbox: point cloud visualization widget
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

#include "point_cloud_visualization_widget.h"


namespace MVL {
namespace StereoToolbox {
namespace GUI {


// *********************************************************************
// *                  Point cloud visualization widget                 *
// *********************************************************************
PointCloudVisualizationWidget::PointCloudVisualizationWidget (QWidget *parent)
    : QOpenGLWidget(parent)
{
}

PointCloudVisualizationWidget::~PointCloudVisualizationWidget ()
{
}

void PointCloudVisualizationWidget::initializeGL ()
{
    QOpenGLFunctions *func = QOpenGLContext::currentContext()->functions();
    func->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void PointCloudVisualizationWidget::resizeGL (int w, int h)
{
    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(45.0f, float(w)/float(h), 0.001f, 100.0f);
}

void PointCloudVisualizationWidget::paintGL ()
{
    QOpenGLFunctions *func = QOpenGLContext::currentContext()->functions();
    func->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


} // GUI
} // StereoToolbox
} // MVL

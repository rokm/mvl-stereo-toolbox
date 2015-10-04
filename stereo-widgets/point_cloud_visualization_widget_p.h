/*
 * Stereo Widgets: point cloud visualization widget
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

#ifndef MVL_STEREO_TOOLBOX__WIDGETS__POINT_CLOUD_VISUALIZATION_WIDGET_P_H
#define MVL_STEREO_TOOLBOX__WIDGETS__POINT_CLOUD_VISUALIZATION_WIDGET_P_H


namespace MVL {
namespace StereoToolbox {
namespace Widgets {


class PointCloudVisualizationWidgetPrivate
{
    Q_DISABLE_COPY(PointCloudVisualizationWidgetPrivate)
    Q_DECLARE_PUBLIC(PointCloudVisualizationWidget)

protected:
    PointCloudVisualizationWidgetPrivate (PointCloudVisualizationWidget *parent);
    PointCloudVisualizationWidget * const q_ptr;

protected:
    QMatrix4x4 projectionMatrix;

    // Point-cloud data
    cv::Mat image;
    cv::Mat points;
    int numPoints;
    bool freshData;

    // OpenGL
    QOpenGLShaderProgram shaderProgramPointCloud;

    QOpenGLBuffer vboPoints; // 6 (XYZ+RGB) floats per vertex

    QOpenGLVertexArrayObject vaoPointCloud;
};


} // Widgets
} // StereoToolbox
} // MVL


#endif

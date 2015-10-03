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

#ifndef POINT_CLOUD_VISUALIZATION_WIDGET_H
#define POINT_CLOUD_VISUALIZATION_WIDGET_H

#include <QtWidgets>
#include <opencv2/core.hpp>


namespace MVL {
namespace StereoToolbox {
namespace GUI {


// *********************************************************************
// *                 Point cloud visualization widget                  *
// *********************************************************************
class PointCloudVisualizationWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    PointCloudVisualizationWidget (QWidget * = 0);
    virtual ~PointCloudVisualizationWidget ();

    void setPointCloud (const cv::Mat &, const cv::Mat &);

protected:
    virtual void initializeGL ();
    virtual void resizeGL (int, int);
    virtual void paintGL ();

protected:
    QMatrix4x4 projectionMatrix;

    // Point-cloud data
    cv::Mat image;
    cv::Mat points;
    int numPoints;
    bool freshData;

    // OpenGL
    QOpenGLShaderProgram shaderProgramPointCloud;

    enum {
        PointCloudData_VertexX,
        PointCloudData_VertexY,
        PointCloudData_VertexZ,
        PointCloudData_ColorR,
        PointCloudData_ColorG,
        PointCloudData_ColorB,

        PointCloudData_Size,
    };
    QOpenGLBuffer vboPoints;

    QOpenGLVertexArrayObject vaoPointCloud;
};


} // GUI
} // StereoToolbox
} // MVL


#endif

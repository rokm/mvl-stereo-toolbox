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
    enum RotationConstraint {
        ConstraintNoAxes,
        ConstraintCameraAxes,
        ConstraintObjectAxes,
    };

    void beginRotation (const QPointF &pos, RotationConstraint constraint);
    void doRotation (const QPointF &pos);
    void endRotation ();

    void projectOnSphere (QVector3D &v) const;
    QQuaternion rotationFromMove (const QVector3D &vFrom, const QVector3D &vTo) const;
    QVector3D constrainToAxis (const QVector3D &vector, const QVector3D &axis) const;
    int findNearestConstraintAxis (const QVector3D &vector) const;

protected:
    QMatrix4x4 projectionMatrix;

    // Point-cloud data
    cv::Mat image;
    cv::Mat points;
    int numPoints;
    bool freshData;

    // Trackball
    int width, height;
    const float trackBallRadius;

    QVector3D cameraAxes[3];
    QVector3D objectAxes[3];

    RotationConstraint rotationConstraint;
    int rotationAxisIndex;

    QQuaternion orientation, prevOrientation;
    QVector3D position, prevPosition;

    bool rotationActive;
    QPointF prevRotationPos;

    // OpenGL
    QOpenGLShaderProgram shaderProgramPointCloud;
    QOpenGLBuffer vboPoints; // 6 (XYZ+RGB) floats per vertex
    QOpenGLVertexArrayObject vaoPointCloud;

    QOpenGLShaderProgram shaderProgramVolumetricLine;
    QOpenGLBuffer vboCircle;
    QOpenGLVertexArrayObject vaoCircle;
    int numCircleVertices;
};


} // Widgets
} // StereoToolbox
} // MVL


#endif
